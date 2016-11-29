#include "serialport.h"
#include <Windows.h>

#include <QDebug>

// constructs a serial port reader (will be pushed into a background thread)
SerialPortReader::SerialPortReader(QObject *parent) :
    QObject(parent),
    m_command(SPC_Cmd_GoWaiting), m_isReaderRunning(false), m_fd(0),
    m_isPortSimulated(false), m_simuOutStream(&m_simuOutFile), m_simuInStream(&m_simuInFile),
    m_isLogEnabled(SERPORT_LOG_ENABLED_DEFAULT)
{
    m_sharedBuffer.reserve(SERPORT_SHARED_BUFFER_SIZE);
}

// the port reader (thread) function
void SerialPortReader::startReading()
{
    size_t i;
    DWORD readBytes;
    bool stopReading;
    PortReaderCommand prevCommand = SPC_Cmd_None;

    for (stopReading = false; !stopReading; )
    {
        if (m_command != prevCommand)
        {
            prevCommand = m_command;
        }

        switch (m_command)
        {
        // wait blocked on a semaphore until a new command is sent by SerialPortHandler
        case SPC_Cmd_GoWaiting:
            m_isReaderRunning = false;
            m_waitSemaphore.acquire();
            m_isReaderRunning = true;
            continue; // for()

        // read data from an opened serial port device
        case SPC_Cmd_DoReading:
            m_isReaderRunning = true;
            if (m_isPortSimulated)
            {
                readBytes = m_simuInStream.readRawData(m_localBuffer, SERPORT_LOCAL_BUFFER_SIZE);
            }
            else
            {
                if (!ReadFile(m_fd, m_localBuffer, SERPORT_LOCAL_BUFFER_SIZE, &readBytes, 0))
                {
                    Sleep(SERPORT_READ_FILE_DELAY_MSEC);
                    continue; // for()
                }
            }

            if (!readBytes)
            {
                Sleep(SERPORT_READ_LOOP_DELAY_MSEC);
                continue; // for()
            }

			if ( ! ( m_localBuffer[ 0 ] & SYNC_VALUE ) )
			{
				qDebug() << "Sync needed";
				syncronizeReading();
				continue;
			}

            m_sharedBufferMutex.lock();
            if (readBytes > SERPORT_SHARED_BUFFER_SIZE - (DWORD)m_sharedBuffer.count())
            {
                // ha nincs elég hely a ring bufferben, akkor kiürítjük az egészet
                m_sharedBuffer.clear();
                m_sharedBuffer.reserve(SERPORT_SHARED_BUFFER_SIZE);
            }
            for (i = 0; i < readBytes; i++)
            {
                m_sharedBuffer.append(m_localBuffer[i]);
            }
            m_sharedBufferMutex.unlock();

            emit dataRead(); // emit data available for reading signal
            break;

        // stop reading (exits the reader thread)
        case SPC_Cmd_Exit:
            stopReading = true;
            break;

        default:
            break;
        } // switch
    } // for()

    m_isReaderRunning = false;

    emit exited(); // emit serial port reader thread exited signal
}

// returns string representation of 'PortReaderCommand' type
QString SerialPortReader::PortReaderCommand_toString(PortReaderCommand command)
{
    switch (command)
    {
    case SPC_Cmd_None:      return "SPC_Cmd_None";
    case SPC_Cmd_GoWaiting: return "SPC_Cmd_GoWaiting";
    case SPC_Cmd_DoReading: return "SPC_Cmd_DoReading";
    case SPC_Cmd_Exit:      return "SPC_Cmd_Exit";
    default:                return "UNKNOWN";
    }
}

void SerialPortReader::syncronizeReading()
{
	//while ( true )
	//{
		for each ( char byte in m_localBuffer )
		{
			if ( byte & SYNC_VALUE )
			{
				purgeReadBuffer();
				return;
			}
		}
	//}
}

bool SerialPortReader::purgeReadBuffer()
{
	m_sharedBufferMutex.lock();
	bool purged = m_sharedBuffer.count() > 0;
	m_sharedBuffer.clear();
	m_sharedBuffer.reserve( SERPORT_SHARED_BUFFER_SIZE );
	m_sharedBufferMutex.unlock();
	return purged;
}

// ----- serial port handler ----------------------------------------------------------------------


// constructs a serial port handler
SerialPort::SerialPort(QObject *parent) :
    QObject(parent),
    m_lastError(0)
{
    m_portReader.moveToThread(&m_thread);
    connect(&m_thread,     SIGNAL(started()),  &m_portReader, SLOT(startReading()));
    connect(&m_portReader, SIGNAL(dataRead()), this,          SLOT(portReaderDataRead()));
    connect(&m_portReader, SIGNAL(exited()),   this,          SLOT(portReaderExited()));
    connect(&m_portReader, SIGNAL(exited()),   &m_thread,     SLOT(quit()));
    m_thread.start();
}

// destructor
SerialPort::~SerialPort()
{
    terminate();
}

// opens a real serial port
bool SerialPort::open(QString portName, DCB& dcb)
{
    int ret;

    if (close() == -1)
    {
        return false;
    }

    m_portReader.m_fd = CreateFile(
                            reinterpret_cast<const wchar_t *>(portName.utf16()),
                            GENERIC_READ | GENERIC_WRITE,
                            0, //not shared
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL, //async
                            NULL);
    if (m_portReader.m_fd == INVALID_HANDLE_VALUE)
    {
        m_lastError = getLastError();
        return false; // ERROR
    }

    // DCB (Device Control Block) beallitas
    if(!SetCommState(m_portReader.m_fd, &dcb))
    {
        m_lastError = getLastError();
        close();
        return false;
    }

    //buffermeretek
    SetupComm(m_portReader.m_fd, SERPORT_LOCAL_BUFFER_SIZE, SERPORT_LOCAL_BUFFER_SIZE);

    //timeout-ok
    _COMMTIMEOUTS cto;
    cto.ReadIntervalTimeout=MAXDWORD;
    cto.ReadTotalTimeoutMultiplier=0;
    cto.ReadTotalTimeoutConstant=0;
    cto.WriteTotalTimeoutMultiplier=0;
    cto.WriteTotalTimeoutConstant=0;
    if(!SetCommTimeouts(m_portReader.m_fd, &cto))
    {
        m_lastError = getLastError();
        close();
        return false;
    }

    // wake up serial port reader thread
    ret = sendPortReaderCommand(SPC_Cmd_DoReading);
    if (ret  == -1)
    {
        close();
        return false;
    }

    return true;
}

// opens a simulated serial port
bool SerialPort::open(QString outFileName, QString inFileName)
{
    int ret;

    if (close() == -1)
    {
        m_lastError = getLastError();
        return false;
    }

    m_portReader.m_simuOutFile.setFileName(outFileName);
    if (m_portReader.m_simuOutFile.open(QIODevice::WriteOnly | QIODevice::Truncate) == false)
        return false;

    m_portReader.m_simuInFile.setFileName(inFileName);
    if (m_portReader.m_simuInFile.open(QIODevice::ReadWrite | QIODevice::Truncate) == false)
    {
        m_portReader.m_simuOutFile.close();
        return false;
    }

    ret = sendPortReaderCommand(SPC_Cmd_DoReading);
    if (ret  == -1)
    {
        close();
        return false;
    }

    m_portReader.m_isPortSimulated = true;
    return true;
}

// closes the (real or simulated) serial port
int SerialPort::close()
{
    int ret = 0;

    if (sendPortReaderCommand(SPC_Cmd_GoWaiting) == -1)
    {
        ret = -1;
    }
    if (m_portReader.m_fd > 0)
    {
        CloseHandle(m_portReader.m_fd);
        m_portReader.m_fd = 0;
    }

    if (m_portReader.m_isPortSimulated)
    {
        m_portReader.m_simuOutFile.close();
        m_portReader.m_simuInFile.close();
    }

    m_portReader.m_isPortSimulated = false;

    return ret;
}

// returns true if the serial port is opened
bool SerialPort::isOpen()
{
    return (m_portReader.m_fd != 0);
}

// reads data from serial port into 'buf' of maximum size 'len'; returns the number of bytes read or (-1)
int SerialPort::read(char* buf, size_t len)
{
    DWORD readBytes = 0;
    m_portReader.m_sharedBufferMutex.lock();
    for(size_t i=0; i<len; i++)
    {
        if (m_portReader.m_sharedBuffer.count() == 0)
        {
            break;
        }
        buf[i] = m_portReader.m_sharedBuffer.takeFirst();
        readBytes++;
    }
    m_portReader.m_sharedBufferMutex.unlock();
    return (int)readBytes;
}

// writes data from 'buf' to serial port; returns the number of bytes written or (-1)
int SerialPort::write(const char* buf, size_t len)
{
    DWORD writtenBytes;

    if (m_portReader.m_isPortSimulated)
    {
        writtenBytes = m_portReader.m_simuOutStream.writeRawData(buf, len);
        m_portReader.m_simuOutFile.flush();
    }
    else
    {
        if(!WriteFile(m_portReader.m_fd, buf, len, &writtenBytes, 0))
        {
            m_lastError = getLastError();
            return -1;
        }
        //FlushFileBuffers(m_portReader.m_fd);
    }

    return (int)writtenBytes;
}

// writes data from 'buf' into an internal data buffer to read it back as if it was read from the serial port
void SerialPort::writeLoopbackData(const char *buf, size_t len)
{
    m_portReader.m_sharedBufferMutex.lock();
    for (size_t i = 0; i < len; i++)
        m_portReader.m_sharedBuffer.append(buf[i]);
    m_portReader.m_sharedBufferMutex.unlock();
}

// clears the read buffer; returns true if there was data in it
bool SerialPort::purgeReadBuffer()
{
    m_portReader.m_sharedBufferMutex.lock();
    bool purged = m_portReader.m_sharedBuffer.count() > 0;
    m_portReader.m_sharedBuffer.clear();
    m_portReader.m_sharedBuffer.reserve(SERPORT_SHARED_BUFFER_SIZE);
    m_portReader.m_sharedBufferMutex.unlock();
    return purged;
}

// activate RTS modem line
void SerialPort::setRts()
{
    EscapeCommFunction(m_portReader.m_fd, SETRTS);
}

// clear RTS modem line
void SerialPort::clearRts()
{
    EscapeCommFunction(m_portReader.m_fd, CLRRTS);
}

// returns an error code resulted by the last failed system call
DWORD SerialPort::getLastError()
{
    return m_lastError;
}

// exits the serial port reader thread
int SerialPort::terminate()
{
    int ret = sendPortReaderCommand(SPC_Cmd_Exit);
    m_thread.quit();
    if (ret == 0)
        m_thread.wait();
    return ret;
}

// sends a command to control the serial port reader thread
int SerialPort::sendPortReaderCommand(PortReaderCommand command)
{
    int ret = 0;

    switch (command)
    {
    case SPC_Cmd_GoWaiting:
        m_portReader.m_command = SPC_Cmd_GoWaiting;
        if (m_portReader.m_isReaderRunning)
        {
            ret = waitForPortReaderRunning(false);
        }
        break;
    case SPC_Cmd_DoReading:
        m_portReader.m_command = SPC_Cmd_DoReading;
        if (m_portReader.m_isReaderRunning == false)
        {
            m_portReader.m_waitSemaphore.release();
            ret = waitForPortReaderRunning(true);
        }
        break;
    case SPC_Cmd_Exit:
        m_portReader.m_command = SPC_Cmd_Exit;
        if (m_portReader.m_isReaderRunning == false)
        {
            m_portReader.m_waitSemaphore.release();
            // ret = waitForPortReaderRunning(true);
        }
        break;
    default:
        break;
    }

    return ret;
}

// waits for running/waiting state of the serial port reader thread
int SerialPort::waitForPortReaderRunning(bool isReaderRunning)
{
    for (int i = 0; i < SERPORT_WAIT_READER_STATE_MAX_ROUNDS; ++i)
    {
        if (m_portReader.m_isReaderRunning == isReaderRunning)
        {
            return 0; // state changed
        }
        Sleep(SERPORT_WAIT_READER_STATE_DELAY_MSEC);
    }
    return -1; // timed out
}

// called when the port reader thread has read some data and put it into the shared buffer
void SerialPort::portReaderDataRead()
{
    emit dataAvailable(); // emit signal
}

// called when the port reader thread has finished working
void SerialPort::portReaderExited()
{
}

