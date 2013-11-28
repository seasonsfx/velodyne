// %pacpus:license{
// This file is part of the PACPUS framework distributed under the
// CECILL-C License, Version 1.0.
// %pacpus:license}
/// @author  Gerald Dherbomez <firstname.surname@utc.fr>
/// @version    $Id: ComponentManager.cpp 116 2013-06-25 11:44:25Z kurdejma $

#include <Pacpus/kernel/ComponentFactoryBase.h>
#include <Pacpus/kernel/ComponentManager.h>
#include <Pacpus/kernel/ComponentBase.h>
#include <Pacpus/kernel/Log.h>

using namespace pacpus;

DECLARE_STATIC_LOGGER("pacpus.core.ComponentManager");

ComponentManager * ComponentManager::mInstance = NULL;

ComponentManager * ComponentManager::create()
{
    return getInstance();
}

ComponentManager * ComponentManager::getInstance()
{
    LOG_TRACE("getInstance()");
    LOG_TRACE("before:  mInstance = " << mInstance);

    if (!mInstance)
    {
        LOG_INFO("creating new instance...");
        mInstance = new ComponentManager();
        LOG_DEBUG("mInstance = " << mInstance);
    }

    LOG_TRACE("after:   mInstance = " << mInstance);
    return mInstance;
}

void ComponentManager::destroy()
{
    LOG_TRACE("destroy");

    delete mInstance;
    mInstance = NULL;
}

ComponentManager::ComponentManager()
{
    LOG_TRACE("constructor");

    xmlTree_ = XmlConfigFile::create();
    LOG_DEBUG("component manager was created");
}

ComponentManager::~ComponentManager()
{
    LOG_TRACE("destructor");

    QMutableMapIterator<ComponentMap::key_type, ComponentMap::mapped_type> it(componentMap_);
    while (it.hasNext())
        unRegisterComponent(it.next().key());

    LOG_DEBUG("component manager was deleted");
}

bool ComponentManager::registerComponentFactory(ComponentFactoryBase* addr, const QString& type)
{
    LOG_TRACE("registerComponentFactory(type="<< type << ")");

    if (factoryMap_.contains(type))
    {
        LOG_WARN("cannot register a component factory of type '" << type << "'. It already belongs to the manager");
        return false;
    }

    factoryMap_[type] = addr;
    LOG_INFO("registered component factory '" << type << "'");

    return true;
}

bool ComponentManager::unRegisterComponentFactory(const QString& type)
{
    LOG_TRACE("unRegisterComponentFactory(type="<< type << ")");

    if (!factoryMap_.contains(type))
    {
        LOG_WARN("cannot unregister component factory '" << type << "'. It was not registered");
        return false;
    }

    factoryMap_.remove(type);
    LOG_INFO("unregistered component factory '" << type << "'");

    return true;
}

bool ComponentManager::registerComponent(ComponentBase* addr, const QString& name)
{
    LOG_TRACE("registerComponent(name="<< name << ")");

    if (componentMap_.contains(name))
    {
        LOG_WARN("cannot register component '" << name << "'. A component with the same name exists already");
        return false;
    }

    componentMap_[name] = addr;
    LOG_INFO("registered component " << name);

    return true;
}

bool ComponentManager::unRegisterComponent(const QString& name)
{
    LOG_TRACE("unRegisterComponent(name="<< name << ")");

    if (!componentMap_.contains(name))
    {
        LOG_WARN("cannot unregister component '" << name << "'. It was not registered");
        return false;
    }

    // FIXME: delete component
    //delete componentMap_[name];
    componentMap_.remove(name);
    LOG_INFO("unregistered component '" << name << "'");

    return true;
}

bool ComponentManager::createComponent(const QString& type, const QString& name)
{
    LOG_TRACE("createComponent(type=" << type << ", " << "name="<< name << ")");

    FactoryMap::iterator it = factoryMap_.find(type);
    if (it != factoryMap_.end())
    {
      (*it)->addComponent(name);
      return true;
    }

    LOG_WARN("cannot create component '" << name << "'"
             << ". Component factory for type '" << type << "'"
             << " does not exist or was not registered"
             );
    return false;
}

bool ComponentManager::loadPlugin(const QString& filename)
{
    LOG_TRACE("loadPlugin(filename=" << filename << ")");

    pluginLoader_.setFileName(filename);

    if (!pluginLoader_.load()) {
        LOG_ERROR("cannot load plugin '" << filename << "'"
                  << ". Plugin loader returned error: " << pluginLoader_.errorString()
                  );
        return false;
    }

    QObject * plugin = pluginLoader_.instance();
    if (NULL == plugin) {
        LOG_WARN("cannot create an instance of the plugin '" << filename << "'"
                 << ". Plugin loader returned error: " << pluginLoader_.errorString()
                 );
        return false;
    }
    pluginList_.append(plugin);
    LOG_INFO("loaded plugin '" << qobject_cast<PacpusPluginInterface*>(plugin)->name() << "'"
             << " from file '" << pluginLoader_.fileName() << "'"
             );
    return true;
}

std::size_t ComponentManager::loadComponents(const QString& configFilename)
{
    LOG_TRACE("loadComponents(filename=" << configFilename << ")");

    // load the components tree in memory
    xmlTree_->loadFile(configFilename);

    {
        // Load the plugins containing the components
        QStringList plugins = xmlTree_->getAllPlugins();
        Q_FOREACH (QString plugin, plugins) {
            if (!loadPlugin(plugin)) {
                LOG_WARN("cannot load plugin '" << plugin << "'");
            }
        }
    }

    QStringList componentsNameList = xmlTree_->getAllComponents();
    LOG_DEBUG("components in the config file: '" << componentsNameList.join("|") << "'");

    XmlComponentConfig cfg;

    // First, create all the components in the XML list
    for (QStringList::iterator it = componentsNameList.begin(); it != componentsNameList.end(); ++it) {
        LOG_DEBUG("try to create component '" << (*it) << "'");
        cfg.localCopy(xmlTree_->getComponent(*it));
        QString componentType = cfg.getComponentType();
        QString componentName = cfg.getComponentName();
        // create the component and automatically add it to the component manager list
        if (!createComponent(componentType, componentName)) {
            LOG_ERROR("cannot create component '" << componentName << "'");
            continue;
        }
    }

    int componentsToConfigureCount = componentMap_.count();

    // Second, try to configure the components without regarding the dependencies
    for (QStringList::iterator it = componentsNameList.begin(); it != componentsNameList.end(); ++it) {
        LOG_DEBUG("try to configure component '" << (*it) << "'");
        cfg.localCopy(xmlTree_->getComponent(*it));
        QString componentName = cfg.getComponentName();

        // copy locally the config parameters of the component
        ComponentBase * component = getComponent(componentName);
        if (NULL == component) {
            LOG_WARN("component '" << componentName << "' does not exist");
        } else {
            component->param.localCopy(cfg.qDomElement());
            component->configuration_ = component->configureComponent(cfg);

        }
    } // for

    // Third, if some components requested a delayed configuration, retry
    for (QStringList::iterator it = componentsNameList.begin(); it != componentsNameList.end() ; ++it) {
        cfg.localCopy(xmlTree_->getComponent(*it));
        QString componentName = cfg.getComponentName();

        ComponentBase * component = getComponent(componentName);
        if (NULL == component) {
            LOG_WARN("component '" << componentName << "' does not exist");
        } else {
            if (component->configuration_ == ComponentBase::CONFIGURATION_DELAYED) {
                LOG_DEBUG("try to configure component '" << (*it) << "'");

                // copy locally the config parameters of the component
                component->param.localCopy(cfg.qDomElement());
                component->configuration_ = component->configureComponent(cfg);
            }

            if (component->configuration_ == ComponentBase::CONFIGURED_OK) {
                --componentsToConfigureCount;
            } else {
                LOG_ERROR("cannot configure component '" << (*it) << "'"
                          << ". Dependencies with other components are too complex"
                          << ". It was not configured, please review your configuration and/or your component"
                          );
                component->configuration_ = ComponentBase::CONFIGURED_FAILED;
            }
        }
    } // for

    LOG_INFO(componentMap_.count() << " component(s) were loaded");
    if (componentsToConfigureCount > 0) {
        LOG_WARN(componentsToConfigureCount << " component(s) were not configured");
    }

    return componentMap_.count();
}

bool ComponentManager::start()
{
    LOG_TRACE("start()");

    bool result = true;
    for (ComponentMap::iterator it = componentMap_.begin(), itend = componentMap_.end(); it != itend; ++it )
        result &= start(it.key());

    return result;
}

bool ComponentManager::start(const QString& componentName)
{
    LOG_TRACE("start(component=" << componentName << ")");

    ComponentBase* component = getComponent(componentName);
    if (!component)
    {
        LOG_WARN("cannot start component '" << componentName << "'.  It does not exist!");
        return false;
    }

    LOG_INFO("starting component '" << componentName << "'...");
    if (!component->startComponent())
        LOG_WARN("cannot start component '" << componentName << "'. It can already be started");

    return true;
}

bool ComponentManager::stop()
{
    LOG_TRACE("stop()");

    bool result = true;
    for (ComponentMap::iterator it = componentMap_.begin(); it != componentMap_.end(); ++it)
        result &= stop(it.key());

    return result;
}

bool ComponentManager::stop(const QString& componentName)
{
    LOG_TRACE("stop(component=" << componentName << ")");

    ComponentBase* component = getComponent(componentName);
    if (!component)
    {
        LOG_WARN("cannot stop component '" << componentName << "'" << ". It does not exist");
        return false;
    }

    LOG_INFO("stopping component '" << componentName << "'...");
    if (!component->stopComponent())
        LOG_WARN("cannot stop component '" << componentName << "'" << ". It can be already stopped");

    return true;
}

ComponentBase* ComponentManager::getComponent(const QString& name)
{
    LOG_TRACE("getComponent(name=" << name << ")");

    ComponentMap::iterator it = componentMap_.find(name);
    if (it != componentMap_.end())
      return *it;

    LOG_WARN("cannot retrieve component '" << name << "'" << ". It does not exist");
    return NULL;
}

QStringList ComponentManager::getAllComponentsName() const
{
    LOG_TRACE("getAllComponentsName()");
    return xmlTree_->getAllComponents();
}