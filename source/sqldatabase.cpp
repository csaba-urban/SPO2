#include <sqldatabase.h>

#include <QMessageBox>

const QString SQLDatabase::PatientDatabaseTableName = "patients";
const QString SQLDatabase::StudiesDatabaseTableName = "studies";


SQLDatabase::SQLDatabase()
: mSqlDatabase( QSqlDatabase::addDatabase( "QSQLITE" ) )
{

}

SQLDatabase::~SQLDatabase()
{
	close();
}

bool SQLDatabase::open( const QString& aDatabasePath )
{
	mSqlDatabase.setDatabaseName( aDatabasePath );

	if ( mSqlDatabase.isOpen() )
	{
		return true;
	}

	if ( ! mSqlDatabase.open() )
	{
		mLastError = QString( "Cannot open database connection. Reason: %1" ).arg( mSqlDatabase.lastError().databaseText() );
		return false;
	}

	return true;
}

void SQLDatabase::close()
{
	mSqlDatabase.close();
}

bool SQLDatabase::insertPatient( const Patient& aPatient )
{
	QSqlQuery q( mSqlDatabase );

	q.prepare( QString( "INSERT INTO %1 \
			(				\
			first_name,		\
			middle_name,	\
			last_name,		\
			patient_id,		\
			birth_date,		\
			gender			\
			)				\
			VALUES (?,?,?,?,?,?)" ).arg( PatientDatabaseTableName ) );

	q.addBindValue( aPatient.firstName() );
	q.addBindValue( aPatient.middleName() );
	q.addBindValue( aPatient.lastName() );
	q.addBindValue( aPatient.patientID() );
	q.addBindValue( aPatient.birthDate().toString( "yyyy.MM.dd" ) );
	q.addBindValue( aPatient.gender() );

	QString queryString = q.lastQuery();

	if ( ! q.exec() )
	{
		mLastError = QString( "Failed to insert patient ( %1 ) into the database. Reason: %2" ).arg( aPatient.name(), getQueryError( q ) );
		return false;
	}

	return true;
}

bool SQLDatabase::deletePatient( const QString& aPatientID )
{
	QSqlQuery q( mSqlDatabase );

	q.prepare( QString( "UPDATE %1 SET active = 0 WHERE patient_id = ?;" ).arg( PatientDatabaseTableName ) );

	q.addBindValue( aPatientID );

	if ( !q.exec() )
	{
		mLastError = QString( "Failed to delete patient ( PatientID = %1 ) from the database. Reason: %2" ).arg( aPatientID, getQueryError( q ) );
		return false;
	}

	return true;
}

bool SQLDatabase::getPatientByName( const QString& aFirstName, const QString& aMiddleName, const QString& aLastName, Patient& aPatient )
{
	QSqlQuery q( mSqlDatabase );

	q.prepare( QString( "SELECT * FROM %1 WHERE first_name = ? AND middle_name = ? AND last_name = ? AND active = 1;" ).arg( PatientDatabaseTableName ) );

	q.addBindValue( aFirstName );
	q.addBindValue( aMiddleName );
	q.addBindValue( aLastName );

	QString s = lastQueryToString( q );

	if ( ! q.exec() )
	{
		mLastError = QString( "Failed to query the patient (%1 %2 %3) from the database. Reason: %4" ).arg( aFirstName, aMiddleName, aLastName, getQueryError( q ) );
		return false;
	}

	if ( q.next() )
	{
		getPatientFromQuery( q, aPatient );
	}

	return true;
}

bool SQLDatabase::getPatientById( const QString& aPatientId, Patient& aPatient )
{
	QSqlQuery q( mSqlDatabase );

	q.prepare( QString( "SELECT * FROM %1 WHERE patient_id = ? AND active = 1;" ).arg( PatientDatabaseTableName ) );

	q.addBindValue( aPatientId );
	
	QString s = lastQueryToString( q );

	if ( !q.exec() )
	{
		mLastError = QString( "Failed to query the patient ( with id %1) from the database. Reason: %2" ).arg( aPatientId, getQueryError( q ) );
		return false;
	}

	if ( q.next() )
	{
		getPatientFromQuery( q, aPatient );
	}

	return true;
}

bool SQLDatabase::getAllPatient( QList< Patient >& aPatients )
{
	aPatients.clear();

	QSqlQuery q( mSqlDatabase );

	q.prepare( QString( "SELECT * FROM %1 WHERE active = 1;" ).arg( PatientDatabaseTableName ) );

	if ( !q.exec() )
	{
		mLastError = QString( "Failed to query the patiens from the database. Reason: %1" ).arg( getQueryError( q ) );
		return false;
	}

	while ( q.next() )
	{
		Patient patient;
		getPatientFromQuery( q, patient );

		aPatients.append( patient );
	}

	return true;
}

bool SQLDatabase::insertStudy( const Study& aStudy )
{
	QSqlQuery q( mSqlDatabase );

	q.prepare( QString( "INSERT INTO %1 \
			(							\
			patient_id,					\
			study_datetime,				\
			patient_weight,				\
			patient_height,				\
			patient_smoking_status,		\
			patient_history,			\
			diagnose,					\
			spo2_record					\
			)							\
			VALUES (?,?,?,?,?,?,?,?)" ).arg( StudiesDatabaseTableName ) );

	q.addBindValue( aStudy.patientID() );
	q.addBindValue( aStudy.studyDateTime().toString( "yyyy.MM.dd hh:mm:ss" ) );
	q.addBindValue( aStudy.patientWeight_kg() );
	q.addBindValue( aStudy.patientHeight_cm() );
	q.addBindValue( aStudy.smokingStatus() );
	q.addBindValue( aStudy.patientHistory() );
	q.addBindValue( aStudy.diagnose() );
	q.addBindValue( aStudy.SPO2Record() );

	if ( !q.exec() )
	{
		mLastError = QString( "Failed to insert study ( with patient Id: %1 ) into the database. Reason: %2" ).arg( aStudy.patientID(), getQueryError( q ) );
		return false;
	}

	return true;
}

QString SQLDatabase::getQueryError( const QSqlQuery& aQuery ) const
{
	return aQuery.lastError().databaseText() + " " + aQuery.lastError().driverText();
}

QString SQLDatabase::lastQueryToString( QSqlQuery& aQuery ) const
{
	QString queryString;
	queryString.append( " The SQL statement: " );
	queryString.append( aQuery.lastQuery() );

	if ( aQuery.boundValues().count() > 0 )
	{
		queryString.append( "; bind values:" );
		for ( int i = 0; i < aQuery.boundValues().count(); ++i )
		{
			queryString.append( QString( " " ) + aQuery.boundValue( i ).toString() + ";" );
		}
	}
	return queryString;
}

bool SQLDatabase::getPatientFromQuery( const QSqlQuery& aQuery, Patient& aPatient )
{
	aPatient.setFirstName( aQuery.value( "first_name" ).toString() );
	aPatient.setMiddleName( aQuery.value( "middle_name" ).toString() );
	aPatient.setLastName( aQuery.value( "last_name" ).toString() );
	aPatient.setGenderName( Patient::EGender( aQuery.value( "gender" ).toInt() ) );
	aPatient.setBirthDate( aQuery.value( "birth_date" ).toDate() );
	aPatient.setPatientID( aQuery.value( "patient_id" ).toString() );

	return true;
}
