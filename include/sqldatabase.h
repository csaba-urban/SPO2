#pragma once

#include "patient.h"
#include "study.h"

#include <QString>
#include <QtSql>

class SQLDatabase
{
public:
	SQLDatabase();
	~SQLDatabase();

	/*
		opens a database connection
	*/
	bool open( const QString& aDatabasePath );
	
	/*
		closes the database connection
	*/
	void close();

	/*
		inserts a Patient into the database
	*/
	bool insertPatient( const Patient& aPatient );

	/*
		logically deletes (sets the active property to 0) patient by the patient's ID
	*/
	bool deletePatient( const QString& aPatientID );

	/*
		gets the patient by it's name
	*/
	bool getPatientByName( const QString& aFirstName, const QString& aMiddleName, const QString& aLastName, Patient& aPatient );

	/*
		gets the patient by it's id
	*/
	bool getPatientById( const QString& aPatientId, Patient& aPatient );

	/*
		gets all the patients from the database
	*/
	bool getAllPatient( QList< Patient >& aPatients );

	/*
		inserts a new study into the database
	*/
	bool insertStudy( const Study& aStudy );

	/*
		returns the last error
	*/
	const QString& lastError() { return mLastError; }

private:
	QSqlDatabase mSqlDatabase;
	QString mLastError;

	static const QString PatientDatabaseTableName;
	static const QString StudiesDatabaseTableName;

	QString getQueryError( const QSqlQuery& aQuery ) const;
	QString lastQueryToString( QSqlQuery& aQuery ) const;
	bool getPatientFromQuery( const QSqlQuery& aQuery, Patient& aPatient );
};

