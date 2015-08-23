#ifndef Adb_H
#define Adb_H
 
#include <kio/global.h>
#include <kio/slavebase.h>

using namespace KIO;

/**
  This class implements a Adb-world kioslave
 */
class Adb : public QObject, public KIO::SlaveBase
{
	Q_OBJECT
	private:
		int exec(const QStringList &arguments, QByteArray &read_stdout, QByteArray &read_stderr);
		void splitLsLine(QString line, QString &perm, QString &owner, QString &group, QString &size, QString &date, QString &filename);

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