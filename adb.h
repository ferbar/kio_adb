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

		UDSEntry getEntry( const KUrl& url );
		UDSEntry getEntry( const QString& fullLine );

	public:
		Adb( const QByteArray &pool, const QByteArray &app );
		virtual ~Adb();

		virtual void special ( const QByteArray& data );
		virtual void listDir ( const KUrl& url );
		virtual void stat ( const KUrl& url );
		virtual void mimetype ( const KUrl& url );
		virtual void get ( const KUrl& url );
		virtual void put ( const KUrl& url, int, JobFlags flags );
		virtual void copy ( const KUrl& src, const KUrl& dest, int, JobFlags flags );
		virtual void mkdir ( const KUrl& url, int );
		virtual void del ( const KUrl& url, bool );
		virtual void rename ( const KUrl& src, const KUrl& dest, JobFlags flags );
};
 
#endif
