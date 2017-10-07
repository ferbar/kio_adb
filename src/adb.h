#ifndef Adb_H
#define Adb_H
 
#include <kio/global.h>
#include <kio/slavebase.h>
#include <QProcess>

using namespace KIO;

/**
  This class implements a Adb-world kioslave
 */
class Adb : public QObject, public KIO::SlaveBase
{
	Q_OBJECT
	private:
		int exec(const QStringList &arguments, QByteArray &read_stdout, QByteArray &read_stderr);
		QProcess *exec(const QStringList &arguments);
		void splitLsLine(QString line, QString &perm, QString &owner, QString &group, QString &size, time_t &mtime, QString &filename);

		QString fillArguments(QString fullPath, QStringList &arguments);
		QString removeNewline(QString &line);

		UDSEntry getEntry( const QUrl& url );
		UDSEntry getEntry( const QString& fullLine );

	public:
		Adb( const QByteArray &pool, const QByteArray &app );
		virtual ~Adb();

		virtual void special ( const QByteArray& data );
		virtual void listDir ( const QUrl& url );
		virtual void stat ( const QUrl& url );
		virtual void mimetype ( const QUrl& url );
		virtual void get ( const QUrl& url );
		virtual void put ( const QUrl& url, int, JobFlags flags );
		virtual void copy ( const QUrl& src, const QUrl& dest, int, JobFlags flags );
		virtual void mkdir ( const QUrl& url, int );
		virtual void del ( const QUrl& url, bool );
		virtual void rename ( const QUrl& src, const QUrl& dest, JobFlags flags );

		void error(int _errid, const char *text) { this->error(_errid, QString(text)); };
		void error(int _errid, const QString &text) { this->error(_errid, text); };
};
 
#endif
