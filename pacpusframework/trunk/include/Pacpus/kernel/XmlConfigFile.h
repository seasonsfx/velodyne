// %pacpus:license{
// This file is part of the PACPUS framework distributed under the
// CECILL-C License, Version 1.0.
// %pacpus:license}
/// @file
/// @author  Gerald Dherbomez <firstname.surname@utc.fr>
/// @date    January, 2006
/// @version $Id: XmlConfigFile.h 116 2013-06-25 11:44:25Z kurdejma $
/// @copyright Copyright (c) UTC/CNRS Heudiasyc 2006 - 2013. All rights reserved.
/// @brief Brief description.
///
/// Purpose:    Class that manages the XML file. This file is used to configure permettant d'enregistrer un fichier de 
///             a PACPUS application.
///             The XML file includes 2 sections : 
///             - parameters : parameters of the application
///             - components : a list of components to load

#ifndef DEF_PACPUS_XMLCONFIGFILE_H
#define DEF_PACPUS_XMLCONFIGFILE_H

#include <Pacpus/kernel/PacpusLibConfig.h>
#include <Pacpus/kernel/XmlComponentConfig.h>

#include <QDomElement>
#include <QMutex>
#include <QStringList>

class QFile;

namespace pacpus {

/// XML config properties:
///     list            STRING(S)   name(s) of plugin files to be loaded, separated by pipe symbol '|'
///         e.g. dbt="libDbtPlyGps.so|libDbtPlyVision.so
class PACPUSLIB_API XmlConfigFile
{
    friend XmlComponentConfig::XmlComponentConfig(const QString&);
    friend class ComponentManager;

public:
    /// @todo Documentation
    static XmlConfigFile * create();
    /// @todo Documentation
    static void destroy();
    /// @todo Documentation
    QDomElement getComponent(QString name);
    /// @returns a list of all names of components declared in the XML tree
    QStringList getAllComponents();
    /// @todo Documentation
    QStringList getAllPlugins();
    /// @todo Documentation
    int loadFile(QString fileName);

    /// @todo Documentation
    /// not used
    void saveFile(QString fileName);
    /// @todo Documentation
    /// not used
    void addComponent(QDomElement component);
    /// @todo Documentation
    /// not used
    void delComponent(QDomElement component);

protected:
private:
    XmlConfigFile();
    ~XmlConfigFile();

    static XmlConfigFile * _xmlConfigFile;
    
    QDomElement createComponent(QString name);

private:
    QDomDocument _document;
    QFile * _file;
    QMutex _mutex;

    int _numberOfComponents;
};

} // namespace pacpus

#endif // DEF_PACPUS_XMLCONFIGFILE_H