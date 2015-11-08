#include "adb.h"
#undef QT_NO_DEBUG
#undef KDE_NO_DEBUG_OUTPUT
#include "kdebug.h"
#include <KDebug>
#undef QT_NO_DEBUG
#include <kcomponentdata.h>
#include <kde_file.h>
#include <QCoreApplication>
#include <QProcess>
#include <QFileInfo>
#include <QRegExp>

using namespace KIO;

const char *LS_DATEFORMAT="%Y-%m-%d %H:%M:%S";

// https://techbase.kde.org/Development/Tutorials/KIO_Slaves/Adb_World
// http://api.kde.org/4.10-api/kdelibs-apidocs/kioslave/html/index.html
// http://api.kde.org/4.0-api/kdepimlibs-apidocs/ - ein bissl was zum kioslave
// https://techbase.kde.org/Development/Tutorials/Debugging/Debugging_IOSlaves
// http://www.linuxtopia.org/online_books/linux_desktop_guides/kde_desktop_user_guide/kdebugdialog.html
/**

export KDE_COLOR_DEBUG=1
export KDE_DEBUG_FILELINE=1
kdebugdialog --fullmode

testen mit: kioclient 'cat' 'Adb:///'

*/
// https://github.com/Arakmar/kio-mtp/blob/safeLock/kio_mtp.cpp
// QT docu: http://doc.qt.io/qt-4.8/qstringlist.html

extern "C" int KDE_EXPORT kdemain( int argc, char **argv )
{
	kDebug(7000) << "Entering function";
	KComponentData instance( "kio_Adb" );
	kWarning(7006) << "Warning";
	kError(7000) << "Error";
	qDebug() << "***********************";

	KGlobal::locale();

	QCoreApplication app( argc, argv );

	if (argc != 4)
	{
		fprintf( stderr, "Usage: kio_Adb protocol domain-socket1 domain-socket2\n");
		exit( -1 );
	}
	fprintf(stderr,"kdemain() Adb:// " __DATE__ "\n");
	Adb slave( argv[2], argv[3] );
	slave.dispatchLoop();
	return 0;
}

// kioclient cat 'Adb:///'
void Adb::get( const KUrl &url )
{
	qDebug() << "Entering function Adb::get(" << url.path() << ")";
	/*
	this->mimeType( "text/plain" );
	QByteArray str( "Adb_world " );
	str+=" ";
	str+=url.fileName(); // http://api.kde.org/4.x-api/kdelibs-apidocs/kdecore/html/classKUrl.html
	str+="\n";
	this->data( str );
	this->finished();
	qDebug() << "Leaving function";
	*/

	QStringList arguments;
	arguments << "shell";
	arguments << "cat";
	arguments << url.path() ; // FIXME: escape ?????
	QByteArray read_stdout, read_stderr;
	int rc=this->exec(arguments, read_stdout, read_stderr);
	qDebug() << "-------------------------- cat " << url.fileName() << " rc=" << rc << " -------------------";
	this->data(read_stdout);
	this->finished();
	qDebug() << "Leaving function";
}

void Adb::put ( const KUrl& url, int, JobFlags flags )
{
	qDebug() << "Entering function put(" << url.fileName() << ",," << flags << ")";
}

void Adb::mimetype( const KUrl& url )
{
	qDebug() << "Entering function mimetype(" << url.fileName() << ")";
}

void Adb::stat( const KUrl& url )
{
	qDebug() << "Entering function stat(" << url.path() << ")";
	QStringList arguments;
	arguments << "shell";
	arguments << "ls";
	arguments << "-la"; // hidden files
	QString path=url.path();
	if(path.size()==0) {
		path="/";
	}
	arguments << path ; // FIXME: escape ?????
	QByteArray read_stdout, read_stderr;
	int rc=this->exec(arguments, read_stdout, read_stderr);
	qDebug() << "-------------------------- ls -la " << (path+"") << " rc=" << rc << " -------------------";
	QStringList fileLines = QString(read_stdout).split("\n");
	QString lineFull=fileLines[0];
	UDSEntry entry;
	QString perm, owner, group, size, filename;
	time_t mtime;
	this->splitLsLine(lineFull, perm, owner, group, size, mtime, filename);

	// this->error( ERR_CANNOT_ENTER_DIRECTORY, path);
	entry.insert( KIO::UDSEntry::UDS_NAME, fileLines[0] );
	entry.insert( KIO::UDSEntry::UDS_SIZE, size.toInt() );
	entry.insert( KIO::UDSEntry::UDS_MODIFICATION_TIME, mtime );
	// entry.insert( KIO::UDSEntry::UDS_ACCESS, 0664 );
	// entry.insert( KIO::UDSEntry::UDS_USER, 1000 );
	// entry.insert( KIO::UDSEntry::UDS_GROUP, 1000 );
	// entry.insert ( UDSEntry::UDS_NAME, QLatin1String ( "Adb:///" ) );
	if(perm[0] == 'd') {
		qDebug() << "directory";
		entry.insert ( UDSEntry::UDS_FILE_TYPE, S_IFDIR );
		entry.insert ( UDSEntry::UDS_MIME_TYPE, QLatin1String ( "inode/directory" ) );
		entry.insert ( UDSEntry::UDS_ICON_NAME, QLatin1String ( "drive-removable-media" ) );
	} else if(perm[0] == 'l') {
		QRegExp linkRegex("->\\s*(.*)\\s*$");
		if (linkRegex.indexIn(lineFull) > 0) {
			QStringList list = linkRegex.capturedTexts();
			// das sollt nur [2] sein FIXME error handling
			QString link = list[1];
			entry.insert( UDSEntry::UDS_LINK_DEST, link );
			QStringList arguments;
			arguments << "shell";
			arguments << ( "[ -d " + link + " ] && echo -n true" ) ;
			QByteArray read_stdout, read_stderr;
			rc=this->exec(arguments, read_stdout, read_stderr);
			qDebug() << "link \n["+link+"]";
			qDebug() << "check sagt: " << read_stdout << "\n" << read_stderr;
			if(read_stdout == "true") {
				qDebug() << "softlink isDirectory";
				entry.insert( KIO::UDSEntry::UDS_GUESSED_MIME_TYPE, QString::fromLatin1( "inode/directory" ) );
			}
		} else {
			error(ERR_CANNOT_ENTER_DIRECTORY, "error getting softlink target for "+lineFull);
		}
	} else {
		qDebug() << "file";
		entry.insert ( UDSEntry::UDS_FILE_TYPE, S_IFREG);
	}
	entry.insert ( UDSEntry::UDS_ACCESS, S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH );
	this->statEntry ( entry );
	this->finished();
}


/*
void Adb::printError()
{
	qDebug() << "Got to printError()";

	QByteArray byteArray = myProcess->readAllStandardOutput();
	QByteArray byteArray = myProcess->readAllStandardError();
	/ *
	QStringList strLines = QString(byteArray).split("\n");

	foreach (QString line, strLines){
		ui->e_Log->append(line);
	}
	* /
}
*/

/**
 *
 */
int Adb::exec(const QStringList &arguments, QByteArray &read_stdout, QByteArray &read_stderr)
{
	QString program = "adb";
	QProcess *myProcess = new QProcess(this);
	myProcess->start(program, arguments);
	// connect (myProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(printOutput()));
	// connect (myProcess, SIGNAL(readyReadStandardError()), this, SLOT(printError()));
	bool rc = myProcess->waitForFinished();
	if(!rc) {
		qDebug() << "error waiting for adb";
	}

	read_stdout = myProcess->readAllStandardOutput();
	read_stderr = myProcess->readAllStandardError();
	// qDebug() << read_stdout;
	// qDebug() << read_stderr;
	return myProcess->exitCode();
}

void Adb::splitLsLine(QString line, QString &perm, QString &owner, QString &group, QString &size, time_t &mtime, QString &filename){
	QString sdate, sdate2;
	QTextStream stream(&line);
	stream >> perm >> owner >> group;
	if(perm[0] == '-') { // dir oder link ham keine size!
		stream >> size;
	}
	stream >> sdate >> sdate2 >> filename;

	sdate.append(" ").append(sdate2);
	struct tm tm;
	memset(&tm, 0, sizeof(struct tm));
	strptime(sdate.toAscii(), LS_DATEFORMAT, &tm);
	mtime=mktime(&tm);
	
	qDebug() << ">>>>>>>>>>>>>" << line << "<<<< perm:[" << perm << "] owner:[" << owner << "] group:[" << group << "]" <<
		" size:[" << size << "] date1:[" << sdate << "] date2:[" << sdate2 << "] mtime:[" << mtime << "] filename:[" << filename << "]";
}

void Adb::listDir( const KUrl &url )
{
	qDebug() << "Entering function listDir(" << url.path() << " | " << url.fileName() << ")";
	// zuerst ein normales LS machen, dann hamma für jede datei den dateinamen
	QStringList arguments;
	arguments << "shell";
	arguments << "ls";
	arguments << "-a"; // hidden files
	arguments << url.path() + "/" ; // FIXME: escape ?????
	QByteArray read_stdout, read_stderr;
	int rc=this->exec(arguments, read_stdout, read_stderr);
	qDebug() << "-------------------------- ls -a rc=" << rc << " -------------------";
	QStringList fileLines = QString(read_stdout).split("\n");
	
	// und jetzt das selbe noch einmal mit ls -la damit ma auch mod ham:
	arguments.replace(2,"-la");
	rc=this->exec(arguments, read_stdout, read_stderr);
	qDebug() << read_stdout;
	qDebug() << "-------------------------- ls -l rc=" << rc << " -------------------";
	QStringList fileLinesFull = QString(read_stdout).split("\n");

	if(fileLines.size() != fileLinesFull.size() ) {
		qDebug() << "we have a problem now ...";
		// FIXME: return error
		this->finished();
		return;
	}


	this->totalSize(fileLines.size());

	UDSEntry entry;

	// foreach (QString line, fileLines) {
	for(int i = 0 ; i < fileLines.size(); i++ ) {
		QString line=fileLines[i];
		QString lineFull=fileLinesFull[i];
		if(line.size() >= 1 && line[line.size() - 1] == '\r' ) {
			line.truncate(line.size() - 1);
		}
		if(lineFull.size() >= 1 && lineFull[lineFull.size() - 1] == '\r' ) {
			lineFull.truncate(lineFull.size() - 1);
		}

		QString perm, owner, group, size, filename;
		time_t mtime;
		this->splitLsLine(lineFull, perm, owner, group, size, mtime, filename);

		if(perm[0] == 'd') {
			// entry.insert ( UDSEntry::UDS_ICON_NAME, QLatin1String ( "drive-removable-media" ) );
			entry.insert( UDSEntry::UDS_MIME_TYPE, QLatin1String ( "inode/directory" ) );
			entry.insert( UDSEntry::UDS_FILE_TYPE, S_IFDIR );
		} else if(perm[0] == 'l') { // FIXME: now kio crashes eventually ...
			QRegExp linkRegex("->\\s*(.*)\\s*$");
			if (linkRegex.indexIn(lineFull) > 0) {
				QStringList list = linkRegex.capturedTexts();
				// das sollt nur [2] sein FIXME error handling
				QString link = list[1];
				entry.insert( UDSEntry::UDS_LINK_DEST, link );
				QStringList arguments;
				arguments << "shell";
				arguments << ( "[ -d " + link + " ] && echo -n true" ) ;
				QByteArray read_stdout, read_stderr;
				rc=this->exec(arguments, read_stdout, read_stderr);
				qDebug() << "link ["+link+"] check sagt:\n " << read_stdout << "\n" << read_stderr;
				if(read_stdout == "true") {
					qDebug() << "listDir - symlink is directoy";
					entry.insert( KIO::UDSEntry::UDS_GUESSED_MIME_TYPE, QString::fromLatin1( "inode/directory" ) );
				}
			} else {
				this->error(ERR_CANNOT_ENTER_DIRECTORY, "error getting softlink target for "+lineFull);
			}
		} else {
			entry.insert( UDSEntry::UDS_FILE_TYPE, S_IFREG );
			// entry.insert ( UDSEntry::UDS_MIME_TYPE, getMimetype( file->filetype ) );
		}

		// this->error( ERR_CANNOT_ENTER_DIRECTORY, path);
		entry.insert( KIO::UDSEntry::UDS_NAME, line );
		entry.insert( KIO::UDSEntry::UDS_SIZE, size.toInt() );
		entry.insert( KIO::UDSEntry::UDS_MODIFICATION_TIME, mtime );
		entry.insert( KIO::UDSEntry::UDS_ACCESS, 0664 );
		entry.insert( KIO::UDSEntry::UDS_USER, 1000 );
		entry.insert( KIO::UDSEntry::UDS_GROUP, 1000 );

		this->listEntry( entry, false );
		entry.clear();
	}

	// und am ende noch einmal listEnty mit einem leeren element senden
	this->listEntry( entry, true );
	this->finished();
	qDebug() << "Leaving function listDir()";
}

void Adb::copy(const KUrl& src, const KUrl& dst, int, KIO::JobFlags flags)
{
	QStringList arguments;
	int rc=1;
	if( (src.protocol() == QLatin1String("adb") ) && dst.isLocalFile() ) {
		qDebug() << "entering function Adb::copy from device(" << src.path() << " -> " << dst.path() << ")";
		QFileInfo destination ( dst.path() );
		if ( !(flags & KIO::Overwrite) && destination.exists() )
		{
			error( ERR_FILE_ALREADY_EXIST, dst.path() );
			return;
		}

		arguments << "pull";
		arguments << src.path() ; // FIXME: escape ?????
		arguments << dst.path() ; // FIXME: escape ?????
		QByteArray read_stdout, read_stderr;
		rc=this->exec(arguments, read_stdout, read_stderr);
	} else if(src.isLocalFile() && ( dst.protocol() == QLatin1String("adb") ) ) {
		qDebug() << "entering function Adb::copy to device(" << src.path() << " -> " << dst.path() << ")";
		arguments << "push";
		arguments << src.path() ; // FIXME: escape ?????
		arguments << dst.path() ; // FIXME: escape ?????
		QByteArray read_stdout, read_stderr;
		rc=this->exec(arguments, read_stdout, read_stderr);
	} else {
		 error ( ERR_UNSUPPORTED_ACTION, src.path() );
		 return;
	}
	if(rc != 0) {
		kError() << "error copying file";
		// /usr/include/kio/global.h
		// this->error(ERR_COULD_NOT_COPY, src.path());
		this->error( KIO::ERR_COULD_NOT_WRITE, src.fileName() );
	}
	
	this->finished();
}

void Adb::special(QByteArray const&data)
{
	qDebug() << "entering function Adb::special()";
	Q_UNUSED(data);
}

void Adb::mkdir(KUrl const&url, int)
{
	qDebug() << "entering function Adb::mkdir()";
	error ( ERR_COULD_NOT_MKDIR, url.path() );
}

void Adb::del(KUrl const&, bool)
{
	qDebug() << "entering function Adb::del()";
}

void Adb::rename(KUrl const&, KUrl const&, QFlags<KIO::JobFlag>)
{
	qDebug() << "entering function Adb::rename()";
}


Adb::Adb( const QByteArray &pool, const QByteArray &app )
	: SlaveBase( "Adb", pool, app )
{
	qDebug() << "Adb::Adb()";
}

Adb::~Adb()
{
	qDebug() << "Adb::~Adb()";
}
