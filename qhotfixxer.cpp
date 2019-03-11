#include "qhotfixxer.h"
#include <QString>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QByteArray>
#include <QList>
#include <QStringList>
#include <QFileInfo>
#include <QFileInfoList>
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <Windows.h>

struct QHotFixxer::Data
{
    QStringList src_files;
    QStringList dst_paths;
    QList<QByteArray> dst_hashes;
    QStringList keys;
    QStringList keys_desc;
    Data()
    {
        keys << "src=" << "dst=" << "hash=";
        keys_desc
             << "source file"
             << "destination search path (recursive)"
             << "hash (md5) of destination file";
    }
    ~Data()
    {
    }
    enum Key_Type {Src_Key = 0, Dst_Key = 1, Hash_Key = 2};
    void parseArgs()
    {
        foreach (const QString & _arg, QCoreApplication::instance()->arguments())
        {
            QString _lowered_arg = _arg.toLower().replace("\"", "");
            int _i = 0;
            foreach (const QString & _key, keys)
            {
                if (_lowered_arg.startsWith(_key))
                {
                    _lowered_arg = _lowered_arg.right(_lowered_arg.length() - _key.length());
                    break;
                }
                _i++;
            }
            switch (_i)
            {
            case Src_Key:
                src_files << QFileInfo(_lowered_arg).absoluteFilePath();
                break;
            case Dst_Key:
                dst_paths << QDir(_lowered_arg).absolutePath();
                break;
            case Hash_Key:
                dst_hashes << _lowered_arg.toLatin1();
                break;
            }
        }
    }
    QByteArray fileMd5(const QString & _fn)
    {
        QByteArray _md5;
        QFile _file(_fn);
        if (_file.open(QFile::ReadOnly))
        {
            QCryptographicHash _hash(QCryptographicHash::Md5);
            while (!_file.atEnd())
            {
                _hash.addData(_file.read(2000000));
            }
            _file.close();
            _md5 = _hash.result();
        }
        return _md5;
    }
    QString version(const QString & _module_name) const
    {
        QString _str;
        const int _ver_sz = ::GetFileVersionInfoSize(_module_name.utf16(), 0);
        if (_ver_sz)
        {
            char * _ver_data = new char[_ver_sz];
            if (::GetFileVersionInfo(_module_name.utf16(), 0, _ver_sz, _ver_data))
            {
                VS_FIXEDFILEINFO * _info;
                UINT _len;
                if (::VerQueryValue(_ver_data, (LPTSTR)QString("\\").utf16(), (LPVOID *)&_info, &_len))
                {
                    if (_info->dwProductVersionMS == 0 && _info->dwProductVersionLS == 0)
                    {
                        _str =
                            QString::number(HIWORD(_info->dwFileVersionMS)) + "." +
                            QString::number(LOWORD(_info->dwFileVersionMS)) + "." +
                            QString::number(HIWORD(_info->dwFileVersionLS)) + "." +
                            QString::number(LOWORD(_info->dwFileVersionLS));
                    }
                    else
                    {
                        _str =
                            QString::number(HIWORD(_info->dwProductVersionMS)) + "." +
                            QString::number(LOWORD(_info->dwProductVersionMS)) + "." +
                            QString::number(HIWORD(_info->dwProductVersionLS)) + "." +
                            QString::number(LOWORD(_info->dwProductVersionLS));
                    }
                }
            }
            delete [] _ver_data;
        }
        return _str;
    }
    void nextDir(const QString & _dir, bool _scan = false)
    {
        QFileInfoList _files = QDir(_dir).entryInfoList(QStringList() << "*.*", QDir::Files);
        if (!_scan)
        {
            foreach (const QFileInfo & _fn, _files)
            {
                const QString & _file_path = _fn.absoluteFilePath();
                const QString & _file_name = _fn.fileName().toLower();
                foreach (const QString & _src_file, src_files)
                {
                    if (QFileInfo(_src_file).fileName() == _file_name)
                    {
                        const QByteArray & _md5 = fileMd5(_file_path).toHex();
                        const bool _known = dst_hashes.contains(_md5);
                        const QString & _ver = version(_file_path);
                        if (_known)
                        {
                            if (QFile::exists(_src_file))
                            {
                                QFile::remove(_file_path);
                                QFile::copy(_src_file, _file_path);
                                qDebug() << " known" << _file_path;
                                qDebug() << "  hash\t\t" << _md5;
                                if (!_ver.isEmpty())
                                {
                                    qDebug() << "  version\t" << _ver;
                                }
                                qDebug() << "  hotfixxed";
                            }
                            else
                            {
                                qDebug() << " known" << _file_path;
                                qDebug() << "  hash\t\t" << _md5;
                                if (!_ver.isEmpty())
                                {
                                    qDebug() << "  version\t" << _ver;
                                }
                                qDebug() << "  source file not found";
                            }
                        }
                        else
                        {
                            if (QFile::exists(_src_file) && (_md5 == fileMd5(_src_file).toHex()))
                            {
                                qDebug() << " same as source" << _file_path;
                            }
                            else
                            {
                                qDebug() << " unknown" << _file_path;
                            }
                            qDebug() << "  hash\t\t" << _md5;
                            if (!_ver.isEmpty())
                            {
                                qDebug() << "  version\t" << _ver;
                            }
                            qDebug() << "  created\t" << _fn.created();
                            qDebug() << "  modified\t" << _fn.lastModified();
                            qDebug() << "  size\t\t" << _fn.size();
                        }
                        qDebug();
                        break;
                    }
                }
            }
        }
        QStringList _dirs = QDir(_dir).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        if (_dirs.isEmpty())
        {
            return;
        }
        foreach (const QString & _dn, _dirs)
        {
            nextDir(_dir + "/" + _dn, _scan);
        }
    }
};

QHotFixxer::QHotFixxer(QObject * _parent) :
    QObject(_parent), d(new Data())
{
    d->parseArgs();
}

QHotFixxer::~QHotFixxer()
{

}

void QHotFixxer::fix()
{
    qDebug();
    qDebug() << "(c) 2012 hotfixxer by mist poryvaev";
    qDebug() << ("version " + d->version(QCoreApplication::applicationFilePath())).toAscii().data();
    qDebug();
    if (QCoreApplication::instance()->arguments().count() <= 1)
    {
        qDebug() << "keys:";
        for (int _i = 0; _i < qMin(d->keys.count(), d->keys_desc.count()); _i++)
        {
            qDebug() << " " << d->keys[_i].toAscii().data() << d->keys_desc[_i].toAscii().data();
        }
        return;
    }
    qDebug() << "start hotfixxing";
    foreach (const QString & _dir, d->dst_paths)
    {
        d->nextDir(_dir);
    }
    qDebug() << "all done";
    qDebug();
    emit finished();
}
