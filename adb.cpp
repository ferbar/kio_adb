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

// ls - string parserei - super!
// FIXME: der string da oben wird wohl bei jedem android device anders sein ...
const char *opendir_failed="opendir failed, Permission denied";
const char *no_such_file_or_directory="No such file or directory";

// https://techbase.kde.org/Development/Tutorials/KIO_Slaves/Adb_World
// http://api.kde.org/4.10-api/kdelibs-apidocs/kioslave/html/index.html
// http://api.kde.org/4.0-api/kdepimlibs-apidocs/ - ein bissl was zum kioslave
// für die error codes: http://api.kde.org/4.0-api/kdelibs-apidocs/kio/html/namespaceKIO.html
// https://techbase.kde.org/Development/Tutorials/Debugging/Debugging_IOSlaves
// http://www.linuxtopia.org/online_books/linux_desktop_guides/kde_desktop_user_guide/kdebugdialog.html
// 5er api: http://api.kde.org/frameworks-api/frameworks5-apidocs/kio/html/classKIO_1_1SlaveBase.html#a1e50004d94e4e7044325fdeb49cf488d
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
	kWarning(7006) << "test Warning";
	kError(7000) << "test Error";
	qDebug() << "test debug ***********************";

	KGlobal::locale();

	QCoreApplication app( argc, argv );

	if (argc != 4)
	{
		fprintf( stderr, "Usage: kio_Adb protocol domain-socket1 domain-socket2\n");
		exit( -1 );
	}
	fprintf(stderr,"kdemain() Adb:// " __DATE__ "\n");


	// TODO: check if \r\n translation is necessary
	QStringList arguments;

	arguments << "eins 1";
	arguments << "\"zwei 2\"";
	arguments << "muh mäh drei 3";
	QProcess *myProcess = new QProcess(NULL);
	QString program = "/home/chris/tmp/test.sh";
	myProcess->start(program, arguments);
	      myProcess->closeWriteChannel();
		        bool rc = myProcess->waitForFinished();

	QString read_stdout = myProcess->readAllStandardOutput();
	qDebug() << "christest: stdout: rc:"<<rc<<" [" << read_stdout << "]";
	delete(myProcess);


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
	QString path=this->fillArguments(url.path(), arguments);
	// 20160219: beim adb shell werden alle \n (und nur das \n) in \r\n umgewandelt -> binary files hin, replace \r\n -> \n funktioniert!
	// siehe: https://code.google.com/p/android/issues/detail?id=2482
	arguments << "cat";
	arguments << path ; // FIXME: escape ?????
	QProcess *myProcess=this->exec(arguments);
	// connect(myProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(processOutput()));  // connect process signals with your code
	// connect(myProcess, SIGNAL(readyReadStandardError()), this, SLOT(processOutput()));  // same here
	
	QByteArray read_stdout;
	char lastByte=0;
	while(myProcess->waitForReadyRead()) {
		read_stdout.clear();
		if(lastByte=='\r') {
			read_stdout.append("\r");
			lastByte=0;
		}
	    read_stdout.append(myProcess->readAll());
		read_stdout.replace("\r\n","\n");
		if(read_stdout.endsWith("\r")) {
			lastByte='\r';
			read_stdout.chop(1);
		}
		this->data(read_stdout);
		qDebug() << "-------------------------- cat " << path << " reading " << read_stdout.size() << "bytes -------------------";
	}

	int ret = myProcess->exitCode();
	qDebug() << "-------------------------- cat " << path << " rc=" << ret << " -------------------";
	delete(myProcess);
	this->finished();
	qDebug() << "Leaving function";
}

void Adb::put( const KUrl& url, int, JobFlags flags )
{
	qDebug() << "Entering function Adb::put(" << url.fileName() << ",," << flags << ")";
	error ( ERR_UNSUPPORTED_ACTION, "not yet implemented");
}

void Adb::mimetype( const KUrl& url )
{
	qDebug() << "Entering function adb::mimetype(" << url.fileName() << ")";
	error ( ERR_UNSUPPORTED_ACTION, "not yet implemented");
}

QString Adb::removeNewline(QString &line) {
	if(line.size() >= 1 && line[line.size() - 1] == '\r' ) {
		// substring(size - 1 characters
		return line.left(line.size() - 1);
	} else {
		return line;
	}
}

QString Adb::fillArguments(QString fullPath, QStringList &arguments) {
	if(fullPath.startsWith("/su")) {
		arguments << "su";
		arguments << "-c";
		fullPath=fullPath.mid(3); // mid => substr !!!!
	}
	if(fullPath.size()==0) {
		fullPath="/";
	}
	return fullPath;
}

void Adb::stat( const KUrl& url )
{
	qDebug() << "Entering function adb::stat(" << url.path() << ")";
	QStringList arguments;
	arguments << "shell";
	QString path=this->fillArguments(url.path(), arguments);
	arguments << "ls";
	arguments << "-la"; // hidden files
	arguments << path ; // FIXME: escape ?????
	QByteArray read_stdout, read_stderr;
	int rc=this->exec(arguments, read_stdout, read_stderr);
	qDebug() << "-------------------------- ls -la " << (path+"") << " rc=" << rc << " -------------------";
	QStringList fileLines = QString(read_stdout).split("\n");
	QString lineFull=this->removeNewline(fileLines[0]);
	if(fileLines[0].trimmed() == opendir_failed) {
		// FIXME: der string da oben wird wohl bei jedem android device anders sein ...
		this->error(ERR_ACCESS_DENIED, "error entering directory \""+url.path()+"\"");
		return;
	}
	UDSEntry entry;
	QString perm, owner, group, size, filename;
	time_t mtime;
	this->splitLsLine(lineFull, perm, owner, group, size, mtime, filename);

	// this->error( ERR_CANNOT_ENTER_DIRECTORY, path);
	entry.insert( KIO::UDSEntry::UDS_NAME, filename );
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
		QRegExp linkRegex("->\\s*(.*)\\s*$",Qt::CaseSensitive,QRegExp::RegExp2);
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
			qDebug() << "link: ["+link+"]";
			qDebug() << "check sagt: " << read_stdout << "\n" << read_stderr;
			if(read_stdout == "true") {
				qDebug() << "softlink isDirectory";
				entry.insert( KIO::UDSEntry::UDS_GUESSED_MIME_TYPE, QString::fromLatin1( "inode/directory" ) );
			}
			// FIXME: sollt ma da 
			// void SlaveBase::redirection	(	const QUrl & 	_url	)	
			// aufrufen???
		} else {
			qDebug() << "regex match failed for >>>"<<lineFull<< "<<<";
			this->error(ERR_CANNOT_ENTER_DIRECTORY, "error getting softlink target for "+lineFull+" [regex match failed]");
			return;
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
 * delete ned vergessen!
 */
QProcess* Adb::exec(const QStringList &arguments) {
	QString program = "adb";
	QProcess *myProcess = new QProcess(this);
	qDebug() << "Adb::exec " << program << " " << arguments;
	myProcess->start(program, arguments);
	// connect (myProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(printOutput()));
	// connect (myProcess, SIGNAL(readyReadStandardError()), this, SLOT(printError()));
	return myProcess;
}

int Adb::exec(const QStringList &arguments, QByteArray &read_stdout, QByteArray &read_stderr) {
	QProcess *myProcess = this->exec(arguments);
	myProcess->closeWriteChannel();
	bool rc = myProcess->waitForFinished();
	if(!rc) {
		qDebug() << "error waiting for adb";
	}
	read_stdout = myProcess->readAllStandardOutput();
	read_stderr = myProcess->readAllStandardError();
	read_stdout.replace("\r\n","\n");
	// qDebug() << read_stdout;
	// qDebug() << read_stderr;
	int ret = myProcess->exitCode();
	qDebug() << "Adb::exec rc=" << ret;
	delete myProcess;
	return ret;
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
	qDebug() << "Entering function Adb::listDir(" << url.path() << " | " << url.fileName() << ")";
	// zuerst ein normales LS machen, dann hamma für jede datei den dateinamen
	QStringList arguments;
	arguments << "shell";
	QString path=this->fillArguments(url.path(), arguments);
	arguments << "ls";
	arguments << "-a"; int lsOptPos=arguments.count() -1;  // hidden files
	arguments << path + "/" ; // FIXME: escape ?????
	QByteArray read_stdout, read_stderr;
	int rc=this->exec(arguments, read_stdout, read_stderr);
	qDebug() << "-------------------------- ls -a rc=" << rc << " -------------------";
	QStringList fileLines = QString(read_stdout).split("\n");
	qDebug() << "listDir first line: ["<<fileLines[0]<<"]";
	if(fileLines[0].trimmed() == opendir_failed || fileLines[0].contains(no_such_file_or_directory)) {
		this->error(ERR_ACCESS_DENIED, "error entering directory \""+path+"\" "+fileLines[0]);
		return;
	}
	
	// und jetzt das selbe noch einmal mit ls -la damit ma auch mod ham:
	arguments.replace(lsOptPos,"-la");
	rc=this->exec(arguments, read_stdout, read_stderr);
	qDebug() << read_stdout;
	qDebug() << "-------------------------- ls -l rc=" << rc << " -------------------";
	QStringList fileLinesFull = QString(read_stdout).split("\n");

	if(fileLines.size() != fileLinesFull.size() ) {
		qDebug() << "we have a problem now ...";
		// FIXME: return error
		this->error(ERR_ACCESS_DENIED, "internal error .... @\""+path+"\"");
		return;
	}


	this->totalSize(fileLines.size());

	UDSEntry entry;

	// foreach (QString line, fileLines) {
	for(int i = 0 ; i < fileLines.size(); i++ ) {
		int mode=0664;
		QString line=this->removeNewline(fileLines[i]);
		QString lineFull=this->removeNewline(fileLinesFull[i]);

		QString perm, owner, group, size, filename;
		time_t mtime=0;
		this->splitLsLine(lineFull, perm, owner, group, size, mtime, filename);

		if(perm[0] == 'd') {
			// entry.insert ( UDSEntry::UDS_ICON_NAME, QLatin1String ( "drive-removable-media" ) );
			entry.insert( UDSEntry::UDS_MIME_TYPE, QLatin1String ( "inode/directory" ) );
			entry.insert( UDSEntry::UDS_FILE_TYPE, S_IFDIR );
		} else if(perm == "lstat") { // lstat './protect_f' failed: Permission denied  --- softlinks die wir als der aktuelle user nicht anschaun können
			qDebug() << "lstat error ??? " << perm << "";
			QRegExp linkRegex("lstat '(.*)' failed: Permission denied",Qt::CaseSensitive,QRegExp::RegExp2);
			if (linkRegex.indexIn(lineFull) > 0) {
				QStringList list = linkRegex.capturedTexts();
				QString link = list[1];
				entry.insert( UDSEntry::UDS_LINK_DEST, link );
				mtime=0;
				mode=000;
			} else {
				qDebug() << "lstat error no regex match";
			}
		} else if(perm[0] == 'l' ) { // FIXME: now kio crashes eventually ...
			QRegExp linkRegex("->\\s*(.*)\\s*$",Qt::CaseSensitive,QRegExp::RegExp2);
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
					qDebug() << "listDir - symlink is directoy"; // http://api.kde.org/4.x-api/kdelibs-apidocs/kioslave/html/ftp_8cpp_source.html das macht guessed_mume_type
					// entry.insert( KIO::UDSEntry::UDS_GUESSED_MIME_TYPE, QString::fromLatin1( "inode/directory" ) );
					entry.insert( KIO::UDSEntry::UDS_MIME_TYPE, QLatin1String("inode/directory"));
				}
			} else {
				qDebug() << "regex match failed for >>>"<<lineFull<< "<<<";
				this->error(ERR_CANNOT_ENTER_DIRECTORY, "error getting softlink target for \""+lineFull+"\"");
				return;
			}
		} else {
			entry.insert( UDSEntry::UDS_FILE_TYPE, S_IFREG );
			// entry.insert ( UDSEntry::UDS_MIME_TYPE, getMimetype( file->filetype ) );
		}

		// this->error( ERR_CANNOT_ENTER_DIRECTORY, path);
		entry.insert( KIO::UDSEntry::UDS_NAME, line );
		entry.insert( KIO::UDSEntry::UDS_SIZE, size.toInt() );
		entry.insert( KIO::UDSEntry::UDS_MODIFICATION_TIME, mtime );
		entry.insert( KIO::UDSEntry::UDS_ACCESS, mode );
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
	QByteArray read_stdout, read_stderr;
	if( (src.protocol() == QLatin1String("adb") ) && dst.isLocalFile() ) {
		// 20160220: adb pull filename "filename with whitespaces" is broken: saves the file to "filename". in the git version from 201507 this has been fixed
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
		rc=this->exec(arguments, read_stdout, read_stderr);
		qDebug() << "Adb::copy rc=" << rc << " stdout:" << read_stdout << " stderr:" << read_stderr;
		destination.refresh();
		if(! destination.exists() && destination.isFile()) {
			qDebug() << "adb pull failed to copy file";
			rc=1;
		}

	} else if(src.isLocalFile() && ( dst.protocol() == QLatin1String("adb") ) ) {
		qDebug() << "entering function Adb::copy to device(" << src.path() << " -> " << dst.path() << ")";
		arguments << "push";
		arguments << src.path() ; // FIXME: escape ?????
		arguments << dst.path() ; // FIXME: escape ?????
		rc=this->exec(arguments, read_stdout, read_stderr);
	} else {
		 error ( ERR_UNSUPPORTED_ACTION, src.path() );
		 return;
	}
	if(rc != 0) {
		kError() << "error copying file";
		// /usr/include/kio/global.h
		// this->error(ERR_COULD_NOT_COPY, src.path());
		this->error( KIO::ERR_COULD_NOT_WRITE, src.fileName() + " " + read_stderr);
		return;
	} else {
		this->finished();
	}
}

void Adb::special(QByteArray const&data)
{
	qDebug() << "entering function Adb::special()";
	Q_UNUSED(data);
}

void Adb::mkdir(KUrl const&url, int)
{
	qDebug() << "entering function Adb::mkdir()";
	error ( ERR_UNSUPPORTED_ACTION, url.path() );
	// error ( ERR_COULD_NOT_MKDIR, url.path() );
}

void Adb::del(KUrl const& file, bool)
{
	qDebug() << "entering function Adb::del()";
	if(file.protocol() != QLatin1String("adb") ) {
		qDebug() << " Adb::del invalid protocol";
		this->error(ERR_UNSUPPORTED_PROTOCOL,"Adb::del invalid protocol");
		return;
	}

	if(file.path().startsWith("/su")) {
		qDebug() << " Adb::del don't delete files in su mode!";
		this->error(ERR_ACCESS_DENIED,"don't delete files in su mode!");
		return;
	}

	QStringList arguments;
	arguments << "shell";
	QString path=this->fillArguments(file.path(), arguments);
	arguments << "rm";
	arguments << path ; // FIXME: escape ?????
	arguments << "|| echo error";
	QByteArray read_stdout, read_stderr;
	qDebug() << "Adb::del command: " << arguments;
	int rc=this->exec(arguments, read_stdout, read_stderr);
	if(rc != 0 || read_stdout.trimmed() != "") {
		this->error(ERR_WRITE_ACCESS_DENIED , QString("rc:")+rc+" "+read_stdout.trimmed());
	} else {
		this->finished();
	}
}

void Adb::rename(KUrl const& src, KUrl const&, QFlags<KIO::JobFlag>)
{
	qDebug() << "entering function Adb::rename()";
	error ( ERR_UNSUPPORTED_ACTION, src.path() );
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
