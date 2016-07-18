/* This file is part of COVISE.

You can use it under the terms of the GNU Lesser General Public License
version 2.1 or later, see lgpl-2.1.txt.

* License: LGPL 2+ */

#include "oscCatalogMember.h"
#include "OpenScenarioBase.h"
#include "oscSourceFile.h"
#include "oscUtilities.h"

#include <iostream>

#include <xercesc/dom/DOMAttr.hpp>

#include <boost/algorithm/string.hpp>


namespace ba = boost::algorithm;


using namespace OpenScenario;


/*****
 * constructor
 *****/

oscCatalogMember::oscCatalogMember() :
        oscMember()
{

}


/*****
 * initialization static variables
 *****/

oscCatalogMember::CatalogTypeTypeNameMap initFuncCatToType()
{
    //set the typeName for possible catalogTypes
    oscCatalogMember::CatalogTypeTypeNameMap catToType;
    catToType.emplace("driver", "oscDriver");
    catToType.emplace("entity", "oscEntity");
    catToType.emplace("environment", "oscEnvironment");
    catToType.emplace("maneuver", "oscManeuverTypeA");
    catToType.emplace("miscObject", "oscMiscObject");
    catToType.emplace("observer", "oscObserverTypeA");
    catToType.emplace("pedestrian", "oscPedestrian");
    catToType.emplace("routing", "oscRouting");
    catToType.emplace("vehicle", "oscVehicle");

    return catToType;
}

const oscCatalogMember::CatalogTypeTypeNameMap oscCatalogMember::s_catalogTypeToTypeName = initFuncCatToType();


/*****
 * destructor
 *****/

oscCatalogMember::~oscCatalogMember()
{

}



/*****
 * public functions
 *****/

std::vector<bf::path> oscCatalogMember::getXoscFilesFromDirectory(const bf::path &pathToDirectory)
{
    //output vector
    std::vector<bf::path> fileNames;

    try
    {
        if (bf::exists(pathToDirectory))
        {
            if (bf::is_directory(pathToDirectory))
            {
                //bf::recursive_directory_iterator() constructs the end iterator
                for (bf::recursive_directory_iterator it(pathToDirectory); it != bf::recursive_directory_iterator(); it++)
                {
                    bf::path contentName = it->path();

                    if (bf::is_regular_file(contentName))
                    {
                        std::string lowerName = contentName.generic_string();
                        ba::to_lower(lowerName);
                        std::string extension = ".xosc";
                        std::size_t startPos = lowerName.size() - extension.size();

                        if (lowerName.compare(startPos, std::string::npos, extension) == 0)
                        {
                            fileNames.push_back(contentName);
                        }
                    }
                }
            }
            else
            {
                std::cerr << "Warning! " << pathToDirectory << " is not a path to a directory." << std::endl;
            }
        }
        else
        {
            std::cerr << "Warning! File or directory " << pathToDirectory << " do not exist." << std::endl;
        }
    }
    catch (const bf::filesystem_error& fse)
    {
        std::cerr << "getXoscFilesFromDirectory(): " << fse.what() << std::endl;
    }

    return fileNames;
}

void oscCatalogMember::fastReadCatalogObjects(const std::vector<bf::path> &filenames)
{
    OpenScenarioBase *oscBase = new OpenScenarioBase();
    bool validate = owner->getBase()->getValidation();

    for (size_t i = 0; i < filenames.size(); i++)
    {
        xercesc::DOMElement *rootElem = oscBase->getRootElement(filenames[i].generic_string(), m_catalogType, validate);

        if (rootElem)
        {
            std::string rootElemName = xercesc::XMLString::transcode(rootElem->getNodeName());

            if (rootElemName == m_catalogType)
            {
                xercesc::DOMAttr *attribute = rootElem->getAttributeNode(xercesc::XMLString::transcode("refId"));
                if (attribute)
                {
                    SuccessIntVar successIntVar = getIntFromIntAttribute(attribute);
                    if (successIntVar.first)
                    {
                        int objectRefId = successIntVar.second;
                        AvailableObjectsMap::const_iterator found = m_availableObjects.find(objectRefId);

                        if (found != m_availableObjects.end())
                        {
                            std::cerr << "Warning! Object for catalog " << m_catalogType << " with refId " << objectRefId << " from " << filenames[i] << " is ignored." << std::endl;
                            std::cerr << "First appearance from file " << found->second << " is used." << std::endl;
                        }
                        else
                        {
                            m_availableObjects.emplace(objectRefId, filenames[i]);
                        }
                    }
                    else
                    {
                        std::cerr << "Warning! Object for catalog " << m_catalogType << " in " << filenames[i] << " has an invalid refId and can't be used." << std::endl;
                    }
                }
                else
                {
                    std::cerr << "Warning! Can't find an object for catalog " << m_catalogType << " in file " << filenames[i] << " with attribute 'refId'." << std::endl;
                }
            }
        }
    }

    delete oscBase;
}


//
void oscCatalogMember::setCatalogType(const std::string &catalogType)
{
    m_catalogType = catalogType;
}

std::string oscCatalogMember::getCatalogType() const
{
    return m_catalogType;
}


//
void oscCatalogMember::setAvailableObjectsMap(const AvailableObjectsMap &availableObjects)
{
    m_availableObjects = availableObjects;
}

oscCatalogMember::AvailableObjectsMap oscCatalogMember::getAvailableObjectsMap() const
{
    return m_availableObjects;
}

bool oscCatalogMember::addObjToAvailableObjectsMap(const int objectRefId, const bf::path &fileNamePath)
{
    AvailableObjectsMap::const_iterator found = m_availableObjects.find(objectRefId);
    if (found != m_availableObjects.end())
    {
        std::cerr << "Error! Object with refId " << objectRefId << " exists and is defined in file " << found->second << std::endl;
        return false;
    }

    std::pair<AvailableObjectsMap::const_iterator, bool> returnVal = m_availableObjects.emplace(objectRefId, fileNamePath);
    if (returnVal.second == false)
    {
        std::cerr << "Error! Can't insert refId " << objectRefId << " from file " << fileNamePath << "into map of available objects." << std::endl;
        return false;
    }
    else
    {
        return true;
    }
}

bool oscCatalogMember::removeObjFromAvailableObjectsMap(const int objectRefId)
{
    return m_availableObjects.erase(objectRefId);
}

void oscCatalogMember::deleteAvailableObjectsMap()
{
    m_availableObjects.clear();
}

std::string oscCatalogMember::getType(const std::string &typeName)
{
	std::unordered_map<std::string, std::string>::const_iterator it = s_catalogTypeToTypeName.find(typeName);
	return it->second;
}

//
bool oscCatalogMember::fullReadCatalogObjectWithName(const int objectRefId)
{
    AvailableObjectsMap::const_iterator found = m_availableObjects.find(objectRefId);
    if (found == m_availableObjects.end())
    {
        std::cerr << "Error! Object with refId " << objectRefId << " isn't available. No file to read." << std::endl;
        return false;
    }

    bf::path filePath = found->second;
    if (bf::is_regular_file(filePath))
    {
        bool success = false;

        OpenScenarioBase *oscBase = new OpenScenarioBase;

        //in fullReadCatalogObjectWithName no validation should be done,
        //because during fastReadCatalogObjects validation is done
        xercesc::DOMElement *rootElem = oscBase->getRootElement(filePath.generic_string(), m_catalogType, false);
        if (rootElem)
        {
            std::string rootElemName = xercesc::XMLString::transcode(rootElem->getNodeName());

            if (rootElemName == m_catalogType)
            {
                CatalogTypeTypeNameMap::const_iterator found = s_catalogTypeToTypeName.find(m_catalogType);
                if (found != s_catalogTypeToTypeName.end())
                {
                    //sourceFile for objectName
                    oscSourceFile *srcFile = new oscSourceFile();

                    //set variables for srcFile, differentiate between absolute and relative path for catalog object
                    srcFile->setSrcFileHref(filePath);
                    srcFile->setSrcFileName(filePath.filename());
                    srcFile->setPathFromCurrentDirToMainDir(owner->getSource()->getPathFromCurrentDirToMainDir());
                    bf::path absPathToMainDir;
                    bf::path relPathFromMainDir;
                    if (filePath.is_absolute())
                    {
                        //absPathToMainDir is path to the directory with the imported catalog file
                        absPathToMainDir = filePath.parent_path();
                        relPathFromMainDir = bf::path(); // relative path is empty
                    }
                    else
                    {
                        //absPathToMainDir is path to directory with the file with OpenSCENARIO root element
                        absPathToMainDir = owner->getSource()->getAbsPathToMainDir();

                        //relative path is path from directory from absPathToMainDir to the directory with the imported file
                        std::string pathFromExeToMainDir = owner->getParentObj()->getSource()->getPathFromCurrentDirToMainDir().generic_string();
                        std::string tmpRelPathFromMainDir = filePath.parent_path().generic_string();
                        if (pathFromExeToMainDir.empty())
                        {
                            relPathFromMainDir = tmpRelPathFromMainDir;
                        }
                        else
                        {
                            relPathFromMainDir = tmpRelPathFromMainDir.substr(pathFromExeToMainDir.length() + 1);
                        }
                    }
                    srcFile->setAbsPathToMainDir(absPathToMainDir);
                    srcFile->setRelPathFromMainDir(relPathFromMainDir);
                    srcFile->setRootElementName(rootElemName);

                    //object for objectName
                    std::string catalogTypeName = found->second;
                    oscObjectBase *obj = oscFactories::instance()->objectFactory->create(catalogTypeName);
                    if(obj)
                    {
                        obj->initialize(owner->getBase(), NULL, NULL, srcFile);
                        obj->parseFromXML(rootElem, srcFile);
                        //add objectName and object to oscCatalogMember map
                        this->emplace(objectRefId, obj);
                        //add sourcFile to vector
                        owner->getBase()->addToSrcFileVec(srcFile);

                        success = true;
                    }
                    else
                    {
                        std::cerr << "Error! Could not create an object member of type " << catalogTypeName << std::endl;
                        delete srcFile;
                    }
                }
                else
                {
                    std::cerr << "Warning! Can't determine an typeName for catalog " << m_catalogType << std::endl;
                }
            }
        }

        delete oscBase;

        return success;
    }
    else
    {
        std::cerr << "Error! Can't read from " << filePath << std::endl;
        return false;
    }
}

bool oscCatalogMember::fullReadCatalogObjectFromFile(const bf::path &fileNamePath)
{
    bool success = false;

    SuccessIntVar successIntVar = getObjectRefIdFromFile(fileNamePath);
    if (successIntVar.first)
    {
        int objectRefId = successIntVar.second;
        if (addObjToAvailableObjectsMap(objectRefId, fileNamePath))
        {
            if (fullReadCatalogObjectWithName(objectRefId))
            {
                success = true;
            }
        }
    }
    else
    {
        std::cerr << "Warning! Object for catalog " << m_catalogType << " in " << fileNamePath << " has an empty refId and can't be used." << std::endl;
    }

    return success;
}

bool oscCatalogMember::addCatalogObject(oscObjectBase *objectBase)
{
    if (objectBase)
    {
        //get objectRefId
        //every object in a catalog (e.g. driver, vehicle ...) should have a member 'refId' of type int (oscInt)
        int objectRefId;
        oscMember *objectRefIdMember = objectBase->getMembers()["refId"];
        if (objectRefIdMember)
        {
            oscIntValue *objRefIdIntVal = dynamic_cast<oscIntValue *>(objectRefIdMember->getValue());
            objectRefId = objRefIdIntVal->getValue();
        }

        //get fileName and Path for file to write
        bf::path fileNamePath;
        oscSourceFile *objectSrc = objectBase->getSource();
        if (objectSrc)
        {
            fileNamePath = objectSrc->getSrcFileHref();
        }

        if (fileNamePath.empty())
        {
            std::cerr << "Error! Can't determine filename and path to write the object." << std::endl;
            return false;
        }

        return addCatalogObject(objectRefId, objectBase, fileNamePath);
    }
    else
    {
        std::cerr << "Error! Given pointer to object isn't accessible." << std::endl;
        return false;
    }
}

bool oscCatalogMember::addCatalogObject(const int objectRefId, oscObjectBase *objectBase, bf::path &fileNamePath)
{
    bool success = false;

    if (objectBase != NULL && !fileNamePath.empty())
    {
        AvailableObjectsMap::const_iterator foundAvailableObjects = m_availableObjects.find(objectRefId);
        ObjectsInMemoryMap::const_iterator foundObjectsInMemory = this->find(objectRefId);

        if (foundAvailableObjects == m_availableObjects.end())
        {
            if (foundObjectsInMemory == this->end())
            {
                //add objectRefId and fileName to m_availableObjects
                if (addObjToAvailableObjectsMap(objectRefId, fileNamePath))
                {
                    //add objectRefId and objectPtr to oscCatalogMember map (objects in memory)
                    std::pair<ObjectsInMemoryMap::const_iterator, bool> returnValObjInMem = this->emplace(objectRefId, objectBase);
                    if (returnValObjInMem.second == true)
                    {
                        success = true;
                    }
                    else
                    {
                        std::cerr << "Warning! Can't insert object with refId" << objectRefId << " to catalog " << m_catalogType << std::endl;
                    }
                }
            }
            else
            {
                std::cerr << "Warning: Can't add catalog object refId "  << objectRefId << ". An object with this refId is already registered." << std::endl;
            }
        }
        else
        {
            std::cerr << "Warning! Can't add catalog object with refId " << objectRefId << ". Object is read from file " << foundAvailableObjects->second << std::endl;
        }
    }
    else
    {
        std::cerr << "Warning! Can't add catalog object with refId " << objectRefId << ". Empty filename or no pointer to object." << std::endl;
    }

    return success;
}

bool oscCatalogMember::removeCatalogObject(const int objectRefId)
{
    ObjectsInMemoryMap::const_iterator found = this->find(objectRefId);
    if (found != this->end())
    {
        this->erase(found);
        return true;
    }
    else
    {
        std::cerr << "Error! Can't remove object with refId " << objectRefId << " from catalog " << m_catalogType << ". Object not found." << std::endl;
        return false;
    }
}

oscObjectBase *oscCatalogMember::getCatalogObject(const int objectRefId)
{
    ObjectsInMemoryMap::const_iterator found = this->find(objectRefId);
    if (found != this->end())
    {
        return found->second;
    }
    else
    {
        return NULL;
    }
}

void oscCatalogMember::deleteoscCatalogMemberMap()
{
    this->clear();
}

std::string oscCatalogMember::getPath(const int objectRefId)
{
	AvailableObjectsMap::const_iterator found = m_availableObjects.find(objectRefId);
	if (found != m_availableObjects.end())
	{
		return found->second.string();
	}
	else
	{
		return NULL;
	}
}

void oscCatalogMember::writeCatalogToDOM()
{
	for (unordered_map<int, oscObjectBase *>::const_iterator it = begin(); it != end(); it++)
	{
		oscObjectBase *objFromCatalog = it->second;
		if (objFromCatalog)
		{
			xercesc::DOMDocument *objFromCatalogXmlDoc = objFromCatalog->getSource()->getOrCreateXmlDoc();
				
			xercesc::DOMElement *rootElement = objFromCatalogXmlDoc->getDocumentElement();
			objFromCatalog->writeToDOM(rootElement, objFromCatalogXmlDoc);
		}
	}
}

void oscCatalogMember::writeCatalogToDisk()
{
	for (unordered_map<int, oscObjectBase *>::const_iterator it = begin(); it != end(); it++)
	{
		oscObjectBase *objFromCatalog = it->second;
		if (objFromCatalog)
		{
			objFromCatalog->getSource()->writeFileToDisk();
		}
	}
}

/*****
 * private functions
 *****/

oscCatalogMember::SuccessIntVar oscCatalogMember::getObjectRefIdFromFile(const bf::path &fileNamePath)
{
    SuccessIntVar successIntVar = std::make_pair(false, -1);
    OpenScenarioBase *oscBase = new OpenScenarioBase;
    bool validate = owner->getBase()->getValidation();
    xercesc::DOMElement *rootElem = oscBase->getRootElement(fileNamePath.generic_string(), m_catalogType, validate);

    if (rootElem)
    {
        std::string rootElemName = xercesc::XMLString::transcode(rootElem->getNodeName());

        if (rootElemName == m_catalogType)
        {
            //attribute refId is of type int (oscInt) and store the value of objectRefId
            xercesc::DOMAttr *attribute = rootElem->getAttributeNode(xercesc::XMLString::transcode("refId"));
            if (attribute)
            {
                successIntVar = getIntFromIntAttribute(attribute);
            }
        }
    }

    delete oscBase;

    return successIntVar;
}

oscCatalogMember::SuccessIntVar oscCatalogMember::getIntFromIntAttribute(xercesc::DOMAttr *attribute)
{
    SuccessIntVar successIntVar = std::make_pair(false, -1);
    oscMemberValue::MemberTypes memberTypeInt = oscMemberValue::INT;
    oscMemberValue *memberValInt = oscFactories::instance()->valueFactory->create(memberTypeInt);
    bool initializeSuccess = memberValInt->initialize(attribute);
    if (initializeSuccess)
    {
        oscIntValue *objIntVal = dynamic_cast<oscIntValue *>(memberValInt);
        int intVar = objIntVal->getValue();

        successIntVar = std::make_pair(true, intVar);
    }

    return successIntVar;
}

int oscCatalogMember::generateRefId()
{
	int refId = 0;
	ObjectsInMemoryMap::const_iterator found;
	do
	{
		found = this->find(++refId);
	}while(found != this->end());

	return refId;
}
