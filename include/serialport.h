#ifndef SERIALPORT_H
#define SERIALPORT_H

/**************************************************************************************************
 * Serial port handler (SerialPort) with a reader thread used INTERNALLY (SerialPortReader)
 **************************************************************************************************/

#include <QtCore/qglobal.h>

#include <Windows.h>
#include <QObject>
#include <QSemaphore>
#include <QMutex>
#include <QList>
#include <QString>
#include <QThread>
#include <QFile>
#include <QDataStream>

// enable/disable logging of SerialPortReader
#define SERPORT_LOG_ENABLED_DEFAULT false

// parameters for waiting for port data in a read cycle
#define SERPORT_READ_FILE_DELAY_MSEC 100
#define SERPORT_READ_LOOP_DELAY_MSEC 10

// parameters for waiting for serial port reader thread's command execution
#define SERPORT_WAIT_READER_STATE_DELAY_MSEC 100
#define SERPORT_WAIT_READER_STATE_MAX_ROUNDS 30

#define SERPORT_LOCAL_BUFFER_SIZE  5
#define SERPORT_SHARED_BUFFER_SIZE 5

#define SYNC_VALUE 0x80

// ----- serial port reader -----------------------------------------------------------------------


// to control serial port reader thread's execution (SerialPortHandler sends it to SerialPortReader)
enum PortReaderCommand
{
    SPC_Cmd_None,

    SPC_Cmd_GoWaiting,  // sleep on a semaphore until the next command
    SPC_Cmd_DoReading,  // do reading from the serial port device
    SPC_Cmd_Exit        // stop reading and exit the reader thread
};

// provides a port reader (thread) function with some environment; (will be pushed into a background thread)
class SerialPortReader : public QObject
{
    Q_OBJECT

public:
    explicit SerialPortReader(QObject *parent = 0);
    static QString PortReaderCommand_toString(PortReaderCommand command);
	void syncronizeReading();
	bool purgeReadBuffer();

    volatile PortReaderCommand m_command;
    volatile bool m_isReaderRunning;
    QSemaphore m_waitSemaphore;

    volatile HANDLE m_fd;
    QList<char> m_sharedBuffer;
    QMutex m_sharedBufferMutex;

    bool m_isPortSimulated;
    QFile m_simuInFile;
    QDataStream m_simuInStream;
    QFile m_simuOutFile;
    QDataStream m_simuOutStream;

    volatile bool m_isLogEnabled;

public slots:
    void startReading();

signals:
    void dataRead();
    void exited();

protected:
    char m_localBuffer[SERPORT_LOCAL_BUFFER_SIZE];
};


// ----- serial port handler ----------------------------------------------------------------------


// provides all low-level serial port handling functions
class SerialPort : public QObject
{
    Q_OBJECT

public:
    // constructs a serial port handler
    SerialPort(QObject *parent = 0);
    // destructor
    ~SerialPort();
    // opens a real serial port
    bool open(QString portName, DCB& dcb);
    // opens a simulated serial port (uses binary data files for output and input)
    bool open(QString outFileName, QString inFileName);
    // closes the serial port
    int close();
    // returns true if the serial port is opened
    bool isOpen();
    // reads data from serial port into 'buf' of maximum size 'len'; returns the number of bytes read or (-1)
    int read(char* buf, size_t len);
    // writes data from 'buf' to serial port; returns the number of bytes written or (-1)
    int write(const char* buf, size_t len);
    // writes data from 'buf' into an internal data buffer to read it back as if it was read from the serial port
    void writeLoopbackData(const char* buf, size_t len);
    // clears the read buffer; returns true if there was data in it
    bool purgeReadBuffer();
    // activate RTS modem line
    void setRts();
    // clear RTS modem line
    void clearRts();
    // returns an error code resulted by the last failed system call
    DWORD getLastError();

signals:
    void dataAvailable();
    void whenRead(QString);
    void whenWrite(QString);

public slots:
    void portReaderDataRead();
    void portReaderExited();

protected:
    SerialPortReader m_portReader;
    QThread m_thread;
    DWORD m_lastError;

    int sendPortReaderCommand(PortReaderCommand command);
    int waitForPortReaderRunning(bool isReaderRunning);
    int terminate();
};

#endif // SERIALPORT_H
