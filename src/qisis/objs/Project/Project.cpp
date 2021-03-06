
/**
 * @file
 * $Revision: 1.19 $
 * $Date: 2010/03/22 19:44:53 $
 *
 *   Unless noted otherwise, the portions of Isis written by the USGS are
 *   public domain. See individual third-party library and package descriptions
 *   for intellectual property information, user agreements, and related
 *   information.
 *
 *   Although Isis has been used by the USGS, no warranty, expressed or
 *   implied, is made by the USGS as to the accuracy and functioning of such
 *   software and related material nor shall the fact of distribution
 *   constitute any such warranty, and no responsibility is assumed by the
 *   USGS in connection therewith.
 *
 *   For additional information, launch
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html
 *   in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.
 */
#include "IsisDebug.h"
#include "Project.h"

#include <unistd.h>

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QMessageBox>
#include <QMutex>
#include <QMutexLocker>
#include <QProgressBar>
#include <QStringList>
#include <QtDebug>
#include <QXmlStreamWriter>

#include "BundleSolutionInfo.h"
#include "BundleSettings.h"
#include "Camera.h"
#include "Control.h"
#include "ControlList.h"
#include "CorrelationMatrix.h"
#include "Cube.h"
#include "Directory.h"
#include "Environment.h"
#include "FileName.h"
#include "GuiCamera.h"
#include "GuiCameraList.h"
#include "IException.h"
#include "ImageList.h"
#include "ImageReader.h"
#include "IException.h"
#include "ProgressBar.h"
#include "Target.h"
#include "TargetBodyList.h"
#include "WorkOrder.h"
#include "WorkOrderFactory.h"
#include "XmlStackedHandlerReader.h"

namespace Isis {

  /**
   * Create a new Project. This creates a project on disk at /tmp/username_appname_pid.
   */
  Project::Project(Directory &directory, QObject *parent) :
      QObject(parent) {
    //qDebug()<<"Project::Project";
    m_bundleSettings = NULL;
    m_directory = &directory;
    m_projectRoot = NULL;
    m_cnetRoot = NULL;
    m_idToControlMap = NULL;
    m_idToImageMap = NULL;
    m_idToTargetBodyMap = NULL;
    m_idToGuiCameraMap = NULL;
    m_images = NULL;
    m_imageReader = NULL;
    m_warnings = NULL;
    m_workOrderHistory = NULL;
    m_isTemporaryProject = true;

    m_numImagesCurrentlyReading = 0;

    m_mutex = NULL;
    m_imageReadingMutex = NULL;

    m_idToControlMap = new QMap<QString, Control *>;
    m_idToImageMap = new QMap<QString, Image *>;
    m_idToTargetBodyMap = new QMap<QString, TargetBody *>;
    m_idToGuiCameraMap = new QMap<QString, GuiCamera *>;
    m_idToBundleSolutionInfoMap = new QMap<QString, BundleSolutionInfo *>;

    m_name = "Project";

    // Look for old projects
    QDir tempDir = QDir::temp();
    QStringList nameFilters;
    nameFilters.append(Environment::userName() + "_" +
                       QApplication::applicationName() + "_*");
    tempDir.setNameFilters(nameFilters);

    QStringList existingProjects = tempDir.entryList();
    bool crashedPreviously = false;

    foreach (QString existingProject, existingProjects) {
      FileName existingProjectFileName(tempDir.absolutePath() + "/" + existingProject);
      QString pidString = QString(existingProject).replace(QRegExp(".*_"), "");
      int otherPid = pidString.toInt();

      if (otherPid != 0) {
        if ( !QFile::exists("/proc/" + pidString) ) {
          crashedPreviously = true;
          int status = system( ("rm -rf '" +
                                existingProjectFileName.expanded() + "' &").toLatin1().data() );
          if (status != 0) {
            QString msg = "Executing command [rm -rf" + existingProjectFileName.expanded() +
                          "' &] failed with return status [" + toString(status) + "]";
            throw IException(IException::Programmer, msg, _FILEINFO_);
          }
        }
      }
    }

    if (crashedPreviously && false) {
      QMessageBox::information( NULL,
                                QObject::tr("Crashed"),
                                QObject::tr("It appears %1 crashed. We're sorry.").
                                         arg( QApplication::applicationName() ) );
    }


    try {
      QString tmpFolder = QDir::temp().absolutePath() + "/"
            + Environment::userName() + "_"
            + QApplication::applicationName() + "_" + QString::number( getpid() );
      QDir temp(tmpFolder + "/tmpProject");
      m_projectRoot = new QDir(temp);
      //qDebug()<<"          Create temp project";
      createFolders();
    }
    catch (IException &e) {
      throw IException(e, IException::Programmer, "Error creating project folders.", _FILEINFO_);
    }
    catch (std::exception &e) {
      //  e.what()
      throw IException(IException::Programmer,
          tr("Error creating project folders [%1]").arg( e.what() ), _FILEINFO_);
    }

    // image reader
    m_mutex = new QMutex;

    m_imageReader = new ImageReader(m_mutex, false);

    connect( m_imageReader, SIGNAL( imagesReady(ImageList) ),
             this, SLOT( imagesReady(ImageList) ) );

    connect( this, SIGNAL(imagesAdded(ImageList *) ),
             this, SLOT(addTargetsFromImportedImagesToProject(ImageList *) ) );

    connect( this, SIGNAL(imagesAdded(ImageList *) ),
             this, SLOT(addCamerasFromImportedImagesToProject(ImageList *) ) );

    m_images = new QList<ImageList *>;

    m_controls = new QList<ControlList *>;

    m_targets = new TargetBodyList;

    m_guiCameras = new GuiCameraList;

    m_bundleSolutionInfo = new QList<BundleSolutionInfo *>;

    m_warnings = new QStringList;
    m_workOrderHistory = new QList< QPointer<WorkOrder> >;

    m_imageReadingMutex = new QMutex;

    // TODO: ken testing
//    m_bundleSettings = NULL;
//    m_bundleSettings = new BundleSettings();
  }


  /**
   * Clean up the project. This will bring the Project back to a safe on-disk state.
   */
  Project::~Project() {
    if (m_isTemporaryProject) {
      deleteAllProjectFiles();
    }
    else {
      int undoStackIndex = m_undoStack.index();
      int undoStackCleanIndex = m_undoStack.cleanIndex();

      int undoNeededToRestoreDiskState = undoStackIndex;

      for (int i = undoStackIndex - 1; i >= undoStackCleanIndex; i--) {
        const WorkOrder *currentWorkOrder =
              dynamic_cast<const WorkOrder *>( m_undoStack.command(i) );
        if ( currentWorkOrder && currentWorkOrder->modifiesDiskState() ) {
          undoNeededToRestoreDiskState = i;
        }
      }

      if (undoNeededToRestoreDiskState != undoStackIndex) {
        m_undoStack.setIndex(undoNeededToRestoreDiskState);
      }
    }

    if (m_images) {
      foreach (ImageList *imageList, *m_images) {
        foreach (Image *image, *imageList) {
          delete image;
        }

        delete imageList;
      }

      delete m_images;
      m_images = NULL;
    }

    if (m_controls) {
      foreach (ControlList *controlList, *m_controls) {
        foreach (Control *control, *controlList) {
          delete control;
        }

        delete controlList;
      }
      delete m_controls;
      m_controls = NULL;
    }

    if (m_bundleSolutionInfo) {
      foreach (BundleSolutionInfo *bundleSolutionInfo, *m_bundleSolutionInfo) {
        delete bundleSolutionInfo;
      }

      delete m_bundleSolutionInfo;
      m_bundleSolutionInfo = NULL;
    }

    delete m_idToControlMap;
    m_idToControlMap = NULL;

    delete m_idToImageMap;
    m_idToImageMap = NULL;

    delete m_idToTargetBodyMap;
    m_idToTargetBodyMap = NULL;

    delete m_idToGuiCameraMap;
    m_idToGuiCameraMap = NULL;

    delete m_idToBundleSolutionInfoMap;
    m_idToBundleSolutionInfoMap = NULL;

    m_directory = NULL;

    delete m_projectRoot;
    m_projectRoot = NULL;

    delete m_cnetRoot;
    m_cnetRoot = NULL;

    delete m_imageReader;

    delete m_warnings;
    m_warnings = NULL;

    delete m_workOrderHistory;
    m_workOrderHistory = NULL;

    delete m_bundleSettings;
    m_bundleSettings = NULL;
  }


  /**
   * This creates the project root, image root, and control net root directories.
   */
  void Project::createFolders() {
    QDir dir;
    if ( !dir.mkpath( m_projectRoot->path() ) ) {
      warn("Cannot create project directory.");
      throw IException(IException::Io,
                       tr("Unable to create folder [%1] when trying to initialize project")
                         .arg(m_projectRoot->path() ),
                       _FILEINFO_);
    }

    try {
      if ( !dir.mkdir( cnetRoot() ) ) {
        QString msg = QString("Unable to create folder [%1] when trying to initialize project")
                        .arg( cnetRoot() );
        warn(msg);
        throw IException(IException::Io, msg, _FILEINFO_);
      }

      if ( !dir.mkdir( imageDataRoot() ) ) {
        QString msg = QString("Unable to create folder [%1] when trying to initialize project")
                        .arg( imageDataRoot() );
        warn(msg);
        throw IException(IException::Io, msg, _FILEINFO_);
      }

      if ( !dir.mkdir( resultsRoot() ) ) {
        QString msg = QString("Unable to create folder [%1] when trying to initialize project")
                        .arg( resultsRoot() );
        warn(msg);
        throw IException(IException::Io, msg, _FILEINFO_);
      }
      if ( !dir.mkdir( bundleSolutionInfoRoot() ) ) {
        QString msg = QString("Unable to create folder [%1] when trying to initialize project")
                        .arg( bundleSolutionInfoRoot() );
        warn(msg);
        throw IException(IException::Io, msg, _FILEINFO_);
      }
    }
    catch (...) {
      warn("Failed to create project directory structure");
      throw;
    }
  }


  ImageList *Project::createOrRetrieveImageList(QString name) {
    ImageList *result = imageList(name);
    if (!result) {
      result = new ImageList;

      result->setName(name);
      result->setPath(name);

      connect( result, SIGNAL( destroyed(QObject *) ),
               this, SLOT( imageListDeleted(QObject *) ) );
      //TODO  07-29-14  Kim & Tracie commented this out.  Didn't seem like it was necessary.
      //      If problems, need to understand this code better.
   m_images->append(result);
    }
    return result;
  }


  /**
   * Converts the project settings into XML.
   *
   * The format of the project settings is:
   *
   *  <pre>
   *   <project>
   *     <controlNets>
   *       <controlNet name="..." />
   *     </controlNets>
   *   </project>
   *  </pre>
   */
  void Project::save(QXmlStreamWriter &stream, FileName newProjectRoot) const {
    stream.writeStartElement("project");

    stream.writeAttribute("name", m_name);

    if ( !m_controls->isEmpty() ) {
      stream.writeStartElement("controlNets");

      for (int i = 0; i < m_controls->count(); i++) {
        m_controls->at(i)->save(stream, this, newProjectRoot);
      }

      stream.writeEndElement();
    }

    if ( !m_images->isEmpty() ) {
      stream.writeStartElement("imageLists");

      for (int i = 0; i < m_images->count(); i++) {
        m_images->at(i)->save(stream, this, newProjectRoot);
      }

      stream.writeEndElement();
    }

    //  Write general look of gui, including docked widges
//  QVariant geo_data = saveGeometry();
//  QVariant layout_data = saveState();
//
//  stream.writeStartElement("dockRestore");
//  stream.writeAttribute("geometry", geo_data.toString());
//  stream.writeAttribute("state", layout_data.toString());


    // if ( !m_bundleSolutionInfo->isEmpty() ) {
    //   stream.writeStartElement("bundleSolutionInfo");
    // 
    //   for (int i = 0; i < m_bundleSolutionInfo->count(); i++) {
    //     m_bundleSolutionInfo->at(i)->save(stream, this, newProjectRoot);
    //   }
    // 
    //   stream.writeEndElement();
    // }

    // should we write the runtimes of the runs here???
    if (!m_bundleSolutionInfo->isEmpty()) {
      stream.writeStartElement("bundleRuns");
    
      for (int i = 0; i < m_bundleSolutionInfo->count(); i++) {
        m_bundleSolutionInfo->at(i)->runTime();
      }
    
      stream.writeEndElement();
    }

    stream.writeEndElement();
  }


  /**
   * Serialize the work orders into the given XML
   *
   * The format of the history xml is:
   * <pre>
   *   <history>
   *     <workOrder>
   *        ...
   *     </workOrder>
   *     <workOrder>
   *        ...
   *     </workOrder>
   *   </history>
   * </pre>
   */
  void Project::saveHistory(QXmlStreamWriter &stream) const {
    stream.writeStartElement("history");

    foreach (WorkOrder *workOrder, *m_workOrderHistory) {
      if (workOrder) {
        workOrder->save(stream);
      }
    }

    stream.writeEndElement();
  }

  /**
   * Serialize the warnings into the given XML
   *
   * The format of the warnings xml is:
   * <pre>
   *   <warnings>
   *     <warning text="..." />
   *     <warning text="..." />
   *   </warnings>
   * </pre>
   */
  void Project::saveWarnings(QXmlStreamWriter &stream) const {
    stream.writeStartElement("warnings");

    foreach (QString warning, *m_warnings) {
      stream.writeStartElement("warning");
      stream.writeAttribute("text", warning);
      stream.writeEndElement();
    }

    stream.writeEndElement();
  }


  /**
   * Verify that the input fileNames are image files.
   *
   * @param fileNames names of files on disk
   * @returns the files that are images.
   */
  // TODO: thread via ImageReader
  QStringList Project::images(QStringList fileNames) {

    QStringList result;

    foreach (QString fileName, fileNames) {
      try {
        Cube tmp(fileName);
        result.append(fileName);
      }
      catch (IException &) {
      }
    }

    return result;
  }


  /**
   * Get a list of configuration/settings actions related to reading images into this Project.
   *
   * These are things like default opacity, default filled, etc.
   */
  QList<QAction *> Project::userPreferenceActions() {
    return m_imageReader->actions(ImageDisplayProperties::FootprintViewProperties);
  }


  /**
   * Create and return the name of a folder for placing BundleSolutionInfo.
   *
   * TODO: don't know if sentence below is accurate.
   * This can be called from multiple threads, but should only be called by one thread at a time.
   */
  QDir Project::addBundleSolutionInfoFolder(QString folder) {
    QDir bundleSolutionInfoFolder(bundleSolutionInfoRoot());

    if (!bundleSolutionInfoFolder.mkpath(folder)) {
      throw IException(IException::Io,
                       tr("Could not create bundle results directory [%1] in [%2].")
                       .arg(folder).arg(bundleSolutionInfoFolder.absolutePath()),
                       _FILEINFO_);
    }

    bundleSolutionInfoFolder.cd(folder);
    return bundleSolutionInfoFolder;
  }


  /**
   * Create and return the name of a folder for placing control networks.
   *
   * This can be called from multiple threads, but should only be called by one thread at a time.
   */
  QDir Project::addCnetFolder(QString prefix) {
    QDir cnetFolder = cnetRoot();
    prefix += "%1";
    int prefixCounter = 0;

    QString numberedPrefix;
    do {
      prefixCounter++;
      numberedPrefix = prefix.arg( QString::number(prefixCounter) );
    }
    while ( cnetFolder.exists(numberedPrefix) );

    if ( !cnetFolder.mkpath(numberedPrefix) ) {
      throw IException(IException::Io,
          tr("Could not create control network directory [%1] in [%2].")
            .arg(numberedPrefix).arg( cnetFolder.absolutePath() ),
          _FILEINFO_);
    }

    cnetFolder.cd(numberedPrefix);

    m_currentCnetFolder = cnetFolder;

    return cnetFolder;
  }


  /**
   * Add the given Control's to the current project. This will cause the controls to be
   *   saved/restored from disk, Project-related GUIs to display the control, and enable access to
   *   the controls given access to the project.
   */
  void Project::addControl(Control *control) {

    connect( control, SIGNAL( destroyed(QObject *) ),
             this, SLOT( controlClosed(QObject *) ) );
    connect( this, SIGNAL( projectRelocated(Project *) ),
             control, SLOT( updateFileName(Project *) ) );

    createOrRetrieveControlList( FileName( control->fileName() ).dir().dirName() )->append(control);

    (*m_idToControlMap)[control->id()] = control;

    emit controlAdded(control);
  }


  ControlList *Project::createOrRetrieveControlList(QString name) {
    ControlList *result = controlList(name);

    if (!result) {
      result = new ControlList;

      result->setName(name);
      result->setPath(name);

      connect( result, SIGNAL( destroyed(QObject *) ),
               this, SLOT( controlListDeleted(QObject *) ) );

      emit controlListAdded(result);
      m_controls->append(result);
    }

    return result;
  }


  /**
   * Create and return the name of a folder for placing images.
   *
   * This can be called from multiple threads, but should only be called by one thread at a time.
   */
  QDir Project::addImageFolder(QString prefix) {
    QDir imageFolder = imageDataRoot();
    prefix += "%1";
    int prefixCounter = 0;

    QString numberedPrefix;
    do {
      prefixCounter++;
      numberedPrefix = prefix.arg( QString::number(prefixCounter) );
    }
    while ( imageFolder.exists(numberedPrefix) );

    if ( !imageFolder.mkpath(numberedPrefix) ) {
      throw IException(IException::Io,
          tr("Could not create image directory [%1] in [%2].")
            .arg(numberedPrefix).arg( imageFolder.absolutePath() ),
          _FILEINFO_);
    }

    imageFolder.cd(numberedPrefix);

    return imageFolder;
  }


  /**
   * Read the given cube file names as Images and add them to the project.
   */
  void Project::addImages(QStringList imageFiles) {
    if (m_numImagesCurrentlyReading == 0) {
      m_imageReadingMutex->lock();
    }

    m_numImagesCurrentlyReading += imageFiles.count();
    m_imageReader->read(imageFiles);
  }


  void Project::addImages(ImageList newImages) {
    imagesReady(newImages);
  }


  Control *Project::control(QString id) {
    return (*m_idToControlMap)[id];
  }


  /**
   * Add the given BundleSolutionInfo to the current project. This will cause the 
   * BundleSolutionInfo to be saved/restored from disk, Project-related GUIs to display the 
   * BundleSolutionInfo, and enable access to the BundleSolutionInfo given access to the project. 
   */
  void Project::addBundleSolutionInfo(BundleSolutionInfo *bundleSolutionInfo) {

    connect(bundleSolutionInfo, SIGNAL(destroyed(QObject *)),
            this, SLOT(bundleSolutionInfoClosed(QObject *)));//???
    connect(this, SIGNAL(projectRelocated(Project *)),
            bundleSolutionInfo, SLOT(updateFileName(Project *)));//DNE???

    // create bundle results folder
    QString runTime = bundleSolutionInfo->runTime();
    QDir bundleDir = addBundleSolutionInfoFolder(runTime); //???
                                                           // save solution information to a file
    QString bundleFileName = bundleDir.absolutePath() + "/" + "BundleSolutionInfo.hdf";

    bundleSolutionInfo->bundleSettings()->setOutputFilePrefix(bundleDir.absolutePath() + "/");
    bundleSolutionInfo->createH5File(FileName(bundleFileName));

    loadBundleSolutionInfo(bundleSolutionInfo);
  }

  
  void Project::loadBundleSolutionInfo(BundleSolutionInfo *bundleSolutionInfo) {
    m_bundleSolutionInfo->append(bundleSolutionInfo);

    (*m_idToBundleSolutionInfoMap)[bundleSolutionInfo->id()] = bundleSolutionInfo;

    emit bundleSolutionInfoAdded(bundleSolutionInfo);
  }


  /**
   * Returns the directory associated with this Project. The directory is not part of the project
   *   so a non-const pointer is returned and this is okay.
   *
   * @return The Directory that was used to create this Project.
   */
  Directory *Project::directory() const {
    return m_directory;
  }


  /**
   * Open the project at the given path.
   */
  void Project::open(QString projectPathStr) {
    //qDebug()<<"Project::open    projectPathStr = "<<projectPathStr;
    m_isTemporaryProject = false;

    FileName projectPath(projectPathStr);
    XmlHandler handler(this);

    XmlStackedHandlerReader reader;
    reader.pushContentHandler(&handler);
    reader.setErrorHandler(&handler);

    QString projectXmlPath = projectPath.toString() + "/project.xml";
    QFile file(projectXmlPath);
//qDebug()<<"Project::open     before opening project.xml";
    if ( !file.open(QFile::ReadOnly) ) {
      throw IException(IException::Io,
                       QString("Unable to open [%1] with read access")
                       .arg(projectXmlPath),
                       _FILEINFO_);
    }

    QDir oldProjectRoot(*m_projectRoot);
    *m_projectRoot = projectPath.expanded();

    QXmlInputSource xmlInputSource(&file);
//qDebug()<<"Project::open     before parsing project.xml";
    if (!reader.parse(xmlInputSource)) {
      warn(tr("Failed to open project [%1]").arg(projectPath.original()));
    }

    //qDebug()<<"Project::open     before opening history.xml";
    QString projectXmlHistoryPath = projectPath.toString() + "/history.xml";
    QFile historyFile(projectXmlHistoryPath);

    if ( !historyFile.open(QFile::ReadOnly) ) {
      throw IException(IException::Io,
                       QString("Unable to open [%1] with read access")
                               .arg(projectXmlHistoryPath),
                       _FILEINFO_);
    }

    reader.pushContentHandler(&handler);

    QXmlInputSource xmlHistoryInputSource(&historyFile);
//qDebug()<<"Project::open     before parsing history.xml";
    if (!reader.parse(xmlHistoryInputSource)) {
      warn(tr("Failed to read history from project [%1]").arg(projectPath.original()));
    }

    QString projectXmlWarningsPath = projectPath.toString() + "/warnings.xml";
    QFile warningsFile(projectXmlWarningsPath);

//qDebug()<<"Project::open     before opening warnings.xml";
    if (!warningsFile.open(QFile::ReadOnly)) {
      throw IException(IException::Io,
                       QString("Unable to open [%1] with read access")
                       .arg(projectXmlWarningsPath),
                       _FILEINFO_);
    }

    reader.pushContentHandler(&handler);

    QXmlInputSource xmlWarningsInputSource(&warningsFile);
//qDebug()<<"Project::open     before parsing warnings.xml";
    if (!reader.parse(xmlWarningsInputSource)) {
      warn(tr("Failed to read warnings from project [%1]").arg(projectPath.original()));
    }

    QString directoryXmlPath = projectPath.toString() + "/directory.xml";
    QFile directoryFile(directoryXmlPath);
//qDebug()<<"Project::open      before opening directory.xml";

    if (!directoryFile.open(QFile::ReadOnly)) {
      throw IException(IException::Io,
                       QString("Unable to open [%1] with read access")
                       .arg(directoryXmlPath),
                       _FILEINFO_);
    }

    reader.pushContentHandler(&handler);

    QXmlInputSource xmlDirectoryInputSource(&directoryFile);
//qDebug()<<"Project::open  Before parsing xml";
    if (!reader.parse(xmlDirectoryInputSource)) {
      warn(tr("Failed to read GUI state from project [%1]").arg(projectPath.original()));
    }

    QDir bundleRoot(bundleSolutionInfoRoot());
    if (bundleRoot.exists()) {
      // get QFileInfo for each directory in the bundle root
      bundleRoot.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks); // sym links ok???

      QFileInfoList bundleDirs = bundleRoot.entryInfoList();
      for (int dirListIndex = 0; dirListIndex < bundleDirs.size(); dirListIndex++) {
        // get QFileInfo for each file in this directory
        QDir bundleSolutionDir(bundleDirs[dirListIndex].absoluteFilePath());
        bundleSolutionDir.setFilter(QDir::Files | QDir::NoSymLinks); // sym links ok???

        QFileInfoList bundleSolutionFiles = bundleSolutionDir.entryInfoList();
        for (int fileListIndex = 0; fileListIndex < bundleSolutionFiles.size(); fileListIndex++) {
          // if the file is an hdf file with BundleSolutionInfo object, add it to the project tree
          if (bundleSolutionFiles[fileListIndex].fileName().contains("_BundleSolutionInfo.hdf")) {
            QString  absoluteFileName = bundleSolutionFiles[fileListIndex].absoluteFilePath();
            FileName solutionFile(bundleSolutionFiles[fileListIndex].absoluteFilePath());
            loadBundleSolutionInfo(new BundleSolutionInfo(solutionFile));
          }
        }
      }
    }

    emit projectLoaded(this);
    //qDebug()<<"Project::open   After emit projectLoaded";
  }


  QProgressBar *Project::progress() {
    return m_imageReader->progress();
  }


  Image *Project::image(QString id) {
    return (*m_idToImageMap)[id];
  }


  ImageList *Project::imageList(QString name) {
    QListIterator<ImageList *> it(*m_images);

    ImageList *result = NULL;
    while (it.hasNext() && !result) {
      ImageList *list = it.next();

      if (list->name() == name) result = list;
    }

    return result;
  }


  bool Project::isTemporaryProject() const {
    return m_isTemporaryProject;
  }
  

  WorkOrder *Project::lastNotUndoneWorkOrder() {
    WorkOrder *result = NULL;
    QListIterator< QPointer<WorkOrder> > it(*m_workOrderHistory);
    it.toBack();

    while ( !result && it.hasPrevious() ) {
      WorkOrder *workOrder = it.previous();

      if ( !workOrder->isUndone() && !workOrder->isUndoing() ) {
        result = workOrder;
      }
    }

    return result;
  }


  /**
   * Get the project's GUI name
   */
  QString Project::name() const {
    return m_name;
  }


  const WorkOrder *Project::lastNotUndoneWorkOrder() const {
    const WorkOrder *result = NULL;
    QListIterator< QPointer<WorkOrder> > it(*m_workOrderHistory);
    it.toBack();

    while ( !result && it.hasPrevious() ) {
      WorkOrder *workOrder = it.previous();

      if ( !workOrder->isUndone() && !workOrder->isUndoing() ) {
        result = workOrder;
      }
    }

    return result;
  }


  /**
   * Return mutex used for Naif calls.  This method is thread-safe.
   *
   * @author 2012-09-11 Tracie Sucharski
   *
   * @return QMutex*
   */
  QMutex *Project::mutex() {
    return m_mutex;
  }


  /**
   * Get the top-level folder of the project. This is where the project is opened from/saved to.
   */
  QString Project::projectRoot() const {
    return m_projectRoot->path();
  }


  /**
   * Change the project's name (GUI only, doesn't affect location on disk).
   */
  void Project::setName(QString newName) {
    m_name = newName;
    emit nameChanged(m_name);
  }


  QUndoStack *Project::undoStack() {
    return &m_undoStack;
  }


  QString Project::nextImageListGroupName() {
    int numLists = m_images->size();
    QString maxName = "";
    QString newGroupName = "Group";

    foreach (ImageList *imageList, *m_images) {
      QString name = imageList->name();
      if ( !name.contains("Group") ) continue;
      if ( maxName.isEmpty() ) {
        maxName = name;
      }
      else if (name > maxName) {
        maxName = name;
      }
    }

    if ( maxName.isEmpty() ) {
      newGroupName += QString::number(numLists+1);
    }
    else {
      int maxNum = maxName.remove("Group").toInt();
      maxNum++;

      newGroupName += QString::number(maxNum);
    }
    return newGroupName;

  }


//   void Project::removeImages(ImageList &imageList) {
//     foreach (Image *image, imageList) {
//       removeImage(image);
//     }
//   }


  void Project::waitForImageReaderFinished() {
    QMutexLocker locker(m_imageReadingMutex);
  }


  /**
   * Get the entire list of work orders that have executed.
   */
  QList<WorkOrder *> Project::workOrderHistory() {
    QList<WorkOrder *> result;
    foreach (WorkOrder *workOrder, *m_workOrderHistory) {
      result.append(workOrder);
    }

    return result;
  }


  /**
   * Appends the root directory name 'cnets' to the project.
   *
   * @return The path to the root directory of the cnet data.
   */
  QString Project::cnetRoot(QString projectRoot) {
    return projectRoot + "/cnets";
  }


  /**
   * Get where control networks ought to be stored inside the project. This is a full path.
   *
   * @return The path to the root directory of the cnet data.
   */
  QString Project::cnetRoot() const {
    return cnetRoot( m_projectRoot->path() );
  }


  QList<ControlList *> Project::controls() {

    return *m_controls;
  }


  ControlList *Project::controlList(QString name) {
    QListIterator< ControlList * > it(*m_controls);

    ControlList *result  = NULL;
    while (it.hasNext() && !result) {
      ControlList *list = it.next();

      if (list->name() == name) result = list;
    }

    return result;
  }


  /**
   * Appends the root directory name 'images' to the project .
   *
   * @return The path to the root directory of the image data.
   */
  QString Project::imageDataRoot(QString projectRoot) {
    return projectRoot + "/images";
  }


  /**
   * Accessor for the root directory of the image data.
   *
   * @return The path to the root directory of the image data.
   */
  QString Project::imageDataRoot() const {
    return imageDataRoot( m_projectRoot->path() );
  }


  QList<ImageList *> Project::images() {
    return *m_images;
  }


  /**
   * Appends the root directory name 'targets' to the project .
   *
   * @return The path to the root directory of the target body data.
   */
  QString Project::targetBodyRoot(QString projectRoot) {
    return projectRoot + "/targets";
  }


  /**
   * Accessor for the root directory of the target body data.
   *
   * @return The path to the root directory of the target body data.
   */
  QString Project::targetBodyRoot() const {
    return targetBodyRoot( m_projectRoot->path() );
  }


  TargetBodyList Project::targetBodies() {
    return *m_targets;
  }


  /**
   * Appends the root directory name 'results' to the project.
   *
   * @return The path to the root directory of bundleresults data.
   */
  QString Project::resultsRoot(QString projectRoot) {
    return projectRoot + "/results";
  }


  /**
   * Accessor for the root directory of the results data.
   *
   * @return The path to the root directory of the results data.
   */
  QString Project::resultsRoot() const {
    return resultsRoot( m_projectRoot->path() );
  }


  QList<BundleSolutionInfo *> Project::bundleSolutionInfo() {
    return *m_bundleSolutionInfo;
  }


  /**
   * Appends the root directory name 'bundle' to the project results directory.
   *
   * @return The path to the root directory of bundle results data.
   */
  QString Project::bundleSolutionInfoRoot(QString projectRoot) {
    return projectRoot + "/results/bundle";
  }


  /**
   * Accessor for the root directory of the results data.
   *
   * @return The path to the root directory of the results data.
   */
  QString Project::bundleSolutionInfoRoot() const {
    return bundleSolutionInfoRoot( m_projectRoot->path() );
  }


  /**
   * Delete all of the files, that this project stores, from disk.
   */
  void Project::deleteAllProjectFiles() {
    foreach (ImageList *imagesInAFolder, *m_images) {
      imagesInAFolder->deleteFromDisk(this);
    }

    if ( !m_projectRoot->rmdir( imageDataRoot() ) ) {
      warn( tr("Did not properly clean up images folder [%1] in project").arg( imageDataRoot() ) );
    }

    if ( !m_projectRoot->rmdir( cnetRoot() ) ) {
      warn( tr("Did not properly clean up control network folder [%1] in project")
             .arg( cnetRoot() ) );
    }

    if ( !m_projectRoot->rmdir( resultsRoot() ) ) {
      warn( tr("Did not properly clean up results folder [%1] in project")
             .arg( resultsRoot() ) );
    }

    if ( !m_projectRoot->rmpath( m_projectRoot->path() ) ) {
      warn( tr("Did not properly clean up project in [%1]").arg( m_projectRoot->path() ) );
    }
  }


  /**
   * This is called when the project is moved.
   *
   * @param newProjectRoot The new root directory for the project.
   */
  void Project::relocateProjectRoot(QString newProjectRoot) {
    *m_projectRoot = newProjectRoot;
    emit projectRelocated(this);
  }


  void Project::save() {
    if (m_isTemporaryProject) {
      QString newDestination = QFileDialog::getSaveFileName(NULL, 
                                                            QString("Project Location"), 
                                                            QString("."));

      if ( !newDestination.isEmpty() ) {
        save( QFileInfo(newDestination + "/").absolutePath() );

        // delete the temporary project
        deleteAllProjectFiles();
        relocateProjectRoot(newDestination);
        m_isTemporaryProject = false;
      }
    }
    else {
      save(m_projectRoot->absolutePath(), false);
    }
  }


  void Project::save(FileName newPath, bool verifyPathDoesntExist) {
    if ( verifyPathDoesntExist && QFile::exists( newPath.toString() ) ) {
      throw IException(IException::Io,
                       QString("Projects may not be saved to an existing path [%1]; "
                               "please select a new path or delete the current folder")
                       .arg(newPath.original()),
                       _FILEINFO_);
    }

    QDir dir;
    if (!dir.mkpath(newPath.toString())) {
      throw IException(IException::Io,
                       QString("Unable to save project at [%1] "
                               "because we could not create the folder")
                       .arg(newPath.original()),
                       _FILEINFO_);
    }

    QFile projectSettingsFile(newPath.toString() + "/project.xml");
    if (!projectSettingsFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
      throw IException(IException::Io,
                       QString("Unable to save project at [%1] because the file [%2] "
                               "could not be opened for writing")
                       .arg(newPath.original()).arg(projectSettingsFile.fileName()),
                       _FILEINFO_);
    }

    QXmlStreamWriter writer(&projectSettingsFile);
    writer.setAutoFormatting(true);

    writer.writeStartDocument();

    // Do amazing, fantastical stuff here!!!
    save(writer, newPath);

    writer.writeEndDocument();

    QFile projectHistoryFile(newPath.toString() + "/history.xml");
    if (!projectHistoryFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
      throw IException(IException::Io,
                       QString("Unable to save project at [%1] because the file [%2] "
                               "could not be opened for writing")
                       .arg(newPath.original()).arg(projectHistoryFile.fileName()),
                       _FILEINFO_);
    }

    QXmlStreamWriter historyWriter(&projectHistoryFile);
    historyWriter.setAutoFormatting(true);

    historyWriter.writeStartDocument();
    saveHistory(historyWriter);
    historyWriter.writeEndDocument();

    QFile projectWarningsFile(newPath.toString() + "/warnings.xml");
    if (!projectWarningsFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
      throw IException(IException::Io,
                       QString("Unable to save project at [%1] because the file [%2] could not be "
                               "opened for writing")
                       .arg(newPath.original()).arg(projectWarningsFile.fileName()),
                       _FILEINFO_);
    }

    QXmlStreamWriter warningsWriter(&projectWarningsFile);
    warningsWriter.setAutoFormatting(true);

    warningsWriter.writeStartDocument();
    saveWarnings(warningsWriter);
    warningsWriter.writeEndDocument();

    QFile directoryStateFile(newPath.toString() + "/directory.xml");
    if (!directoryStateFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
      throw IException(IException::Io,
                       QString("Unable to save project at [%1] because the file [%2] could not be "
                               "opened for writing")
                       .arg(newPath.original()).arg(directoryStateFile.fileName()),
                       _FILEINFO_);
    }

    QXmlStreamWriter directoryStateWriter(&directoryStateFile);
    directoryStateWriter.setAutoFormatting(true);

    directoryStateWriter.writeStartDocument();
    //qDebug()<<"Project::save Before save Directory";
    /*
     * TODO: Does Project need to know about Directory?
     * This is the only place that project uses m_directory. This makes me wonder if it is
     * necessary for project to have a Directory member variable.
     */
    m_directory->save(directoryStateWriter, newPath);
    //qDebug()<<"Project::save After save Directory";
    directoryStateWriter.writeEndDocument();
  }


  /**
   * Run the work order and stores it in the project. If WorkOrder::execute() returns true then
   *   the work order's redo is called. This takes ownership of workOrder.
   *
   * The order of events is:
   *   1) WorkOrder::execute()
   *   2) emit workOrderStarting()
   *   3) WorkOrder::redo() [optional - see WorkOrder]
   *
   * @param workOrder The work order to be executed. This work order must not already be in the
   *                    project.
   */
  void Project::addToProject(WorkOrder *workOrder) {
    if (workOrder) {
      connect(workOrder, SIGNAL(finished(WorkOrder *)),
              this, SIGNAL(workOrderFinished(WorkOrder *)));

      workOrder->setPrevious(lastNotUndoneWorkOrder());

      if (workOrder->execute()) {
        if (workOrder->previous()) workOrder->previous()->setNext(workOrder);

        m_workOrderHistory->append(workOrder);

        emit workOrderStarting(workOrder);

        // Work orders that create clean states (save, save as) don't belong on the undo stack.
        //   Instead, we tell the undo stack that we're now clean.
        if (workOrder->createsCleanState()) {
          m_undoStack.setClean();
        }
        else {
          // All other work orders go onto the undo stack
          m_undoStack.push(workOrder); // This calls redo for us
        }

        // Clean up deleted work orders (the m_undoStack.push() can delete work orders)
        m_workOrderHistory->removeAll(NULL);
      }
      else {
        delete workOrder;
        workOrder = NULL;
      }
    }
  }


  template<typename Data> void Project::warn(QString text, Data relevantData) {
    storeWarning(text, relevantData);
    directory()->showWarning(text, relevantData);
  }


  void Project::warn(QString text) {
    foreach (QString line, text.split("\n")) {
      storeWarning(line);
      directory()->showWarning(line);
    }
  }


  void Project::storeWarning(QString text) {
    m_warnings->append(text);
  }


  void Project::imagesReady(ImageList images) {

    m_numImagesCurrentlyReading -= images.count();

    foreach (Image *image, images) {
      connect(image, SIGNAL(destroyed(QObject *)),
              this, SLOT(imageClosed(QObject *)));
      connect(this, SIGNAL(projectRelocated(Project *)),
              image, SLOT(updateFileName(Project *)));

      (*m_idToImageMap)[image->id()] = image;
      createOrRetrieveImageList(FileName(images[0]->fileName()).dir().dirName())->append(image);
    }

    // We really can't have all of the cubes in memory before
    //   the OS stops letting us open more files.
    // Assume cameras are being used in other parts of code since it's
    //   unknown
    QMutexLocker lock(m_mutex);
    emit imagesAdded(m_images->last());

    Image *openImage;
    foreach (openImage, images) {
      openImage->closeCube();
    }

//     if(m_projectPvl && m_projectPvl->HasObject("MosaicFileList") )
//       m_fileList->fromPvl(m_projectPvl->FindObject("MosaicFileList") );

//     if(m_projectPvl && m_projectPvl->HasObject("MosaicScene") )
//       m_scene->fromPvl(m_projectPvl->FindObject("MosaicScene") );

    if (m_numImagesCurrentlyReading == 0) {
      m_imageReadingMutex->unlock();
    }
  }


  void Project::addTargetsFromImportedImagesToProject(ImageList *imageList) {

    bool found = false;
    foreach (Image *image, *imageList) {

      // TODO - I'm a bit worried about being sure the cube is still open at this point (Ken)
      Target *target = image->cube()->camera()->target();

      // construct TargetBody QSharedPointer from this images cameras Target
      TargetBodyQsp targetBody = TargetBodyQsp(new TargetBody(target));

      foreach (TargetBodyQsp tb, *m_targets) {
        if (*tb == *targetBody) {
          found = true;
          break;
        }
      }

      // if this TargetBody is not already in the project, add it
      // below is how it probably should work, would have to I think
      // override the ::contains() method in the TargetBodyList class
//      if (!m_targets->contains(targetBody))
//        m_targets->append(targetBody);

      if (!found) {
        m_targets->append(targetBody);
        connect( targetBody.data(), SIGNAL( destroyed(QObject *) ),
                 this, SLOT( targetBodyClosed(QObject *) ) );
//      connect( this, SIGNAL( projectRelocated(Project *) ),
//               targetBody.data(), SLOT( updateFileName(Project *) ) );

        (*m_idToTargetBodyMap)[targetBody->id()] = targetBody.data();
      }
    }

    emit targetsAdded(m_targets);
  }

  

  void Project::addCamerasFromImportedImagesToProject(ImageList *imageList) {
    bool found = false;
    foreach (Image *image, *imageList) {

      // TODO - I'm a bit worried about being sure the cube is still open at this point (Ken)
      Camera *camera = image->cube()->camera();

      // construct guiCamera QSharedPointer from this images cameras Target
      GuiCameraQsp guiCamera = GuiCameraQsp(new GuiCamera(camera));

      foreach (GuiCameraQsp gc, *m_guiCameras) {
        if (*gc == *guiCamera) {
          found = true;
          break;
        }
      }

      // if this guiCamera is not already in the project, add it
      // below is how it probably should work, would have to I think
      // override the ::contains() method in the GuiCameraList class
//      if (!m_guiCameras->contains(guiCamera))
//        m_guiCameras->append(guiCamera);

      if (!found) {
        m_guiCameras->append(guiCamera);
        connect( guiCamera.data(), SIGNAL( destroyed(QObject *) ),
                 this, SLOT( guiCameraClosed(QObject *) ) );
        connect( this, SIGNAL( projectRelocated(Project *) ),
                 guiCamera.data(), SLOT( updateFileName(Project *) ) );

        (*m_idToGuiCameraMap)[guiCamera->id()] = guiCamera.data();
      }
    }

    emit guiCamerasAdded(m_guiCameras);
  }


  /**
   * An image is being deleted from the project
   */
  void Project::imageClosed(QObject *imageObj) {
    QMutableListIterator<ImageList *> it(*m_images);
    while (it.hasNext()) {
      ImageList *list = it.next();

      int foundElement = list->indexOf((Image *)imageObj);

      if (foundElement != -1) {
        list->removeAt(foundElement);
      }
    }

    m_idToImageMap->remove(m_idToImageMap->key((Image *)imageObj));
  }


  /**
   * A control is being deleted from the project
   */
  void Project::controlClosed(QObject *controlObj) {
    QMutableListIterator<ControlList *> it(*m_controls);
    while ( it.hasNext() ) {
      ControlList *list = it.next();

      int foundElement = list->indexOf( (Control *)controlObj );

      if (foundElement != -1) {
        list->removeAt(foundElement);
      }
    }

    m_idToControlMap->remove(m_idToControlMap->key((Control *)controlObj));
  }


  /**
   * An control list is being deleted from the project.
   */
  void Project::controlListDeleted(QObject *controlListObj) {
    int indexToRemove = m_controls->indexOf(static_cast<ControlList *>(controlListObj));
    if (indexToRemove != -1) {
      m_controls->removeAt(indexToRemove);
    }
  }


  /**
   * An image list is being deleted from the project.
   */
  void Project::imageListDeleted(QObject *imageListObj) {
    int indexToRemove = m_images->indexOf(static_cast<ImageList *>(imageListObj));
    if (indexToRemove != -1) {
      m_images->removeAt(indexToRemove);
    }
  }


  /**
   * A BundleSolutionInfo object is being deleted from the project
   */
  void Project::bundleSolutionInfoClosed(QObject *bundleSolutionInfoObj) {
    QMutableListIterator<BundleSolutionInfo *> it(*m_bundleSolutionInfo);
    while (it.hasNext()) {
      BundleSolutionInfo *bundleSolutionInfo = it.next();
      if (!bundleSolutionInfo) {
        // throw error???
      }

      int foundElement = m_bundleSolutionInfo->indexOf(
          (BundleSolutionInfo *)bundleSolutionInfoObj);

      if (foundElement != -1) {
        m_bundleSolutionInfo->removeAt(foundElement);
      }
    }

    m_idToBundleSolutionInfoMap->remove(
        m_idToBundleSolutionInfoMap->key((BundleSolutionInfo * )bundleSolutionInfoObj));
  }


  /**
   * A target body is being deleted from the project.
   * TODO: should prevent deleting a target body if there are currently images in the project with
   *       this target?
   */
  void Project::targetBodyClosed(QObject *targetBodyObj) {
//    QMutableListIterator<TargetBody *> it(*m_targets);
//    while ( it.hasNext() ) {
//      TargetBody *targetBody = it.next();
//      if (!targetBody) {
//        // throw error???
//      }

//      int foundElement = m_targets->indexOf( (TargetBody *)targetBodyObj );

//      if (foundElement != -1) {
//        m_targets->removeAt(foundElement);
//      }
//    }

//    m_idToTargetBodyMap->remove(m_idToTargetBodyMap->key((TargetBody *)targetBodyObj));
  }



  Project::XmlHandler::XmlHandler(Project *project) {
    m_project = project;
    m_workOrder = NULL;
  }


  bool Project::XmlHandler::startElement(const QString &namespaceURI, const QString &localName,
                                         const QString &qName, const QXmlAttributes &atts) {
    if (XmlStackedHandler::startElement(namespaceURI, localName, qName, atts)) {

      if (localName == "project") {
        QString name = atts.value("name");

        if (!name.isEmpty()) {
          m_project->setName(name);
        }
      }
      else if (localName == "controlNets") {
        m_controls.append(new ControlList(m_project, reader()));
      }
      else if (localName == "imageList") {
        m_imageLists.append(new ImageList(m_project, reader()));
      }
      else if (localName == "workOrder") {
        QString type = atts.value("type");

        m_workOrder = WorkOrderFactory::create(m_project, type);
        ASSERT(m_workOrder->metaObject()->className() == type);

        m_workOrder->read(reader());
      }
      else if (localName == "warning") {
        QString warningText = atts.value("text");

        if (!warningText.isEmpty()) {
          m_project->warn(warningText);
        }
      }
      else if (localName == "directory") {
        m_project->directory()->load(reader());
      }
      else if (localName == "dockRestore") {
//    QVariant geo_data = QVariant(atts.value("geometry"));
//    restoreGeometry(geo_data);
//    QVariant layout_data = QVariant(atts.value("state"));
//    restoreState(layout_data);
      }

      else if (localName == "bundleSettings") {
        BundleSettings *bundleSettings = m_project->bundleSettings();
        if (!bundleSettings) {
          // throw error???
        }
//      bundleSettings = new BundleSettings(m_project, reader());
      }
    }

    return true;
  }


  bool Project::XmlHandler::endElement(const QString &namespaceURI, const QString &localName,
                                       const QString &qName) {
    if (localName == "project") {
      foreach (ImageList *imageList, m_imageLists) {
        m_project->imagesReady(*imageList);
      }
    }
    else if (localName == "workOrder") {
      m_project->m_workOrderHistory->append(m_workOrder);
      m_workOrder = NULL;
    }
    else if (localName == "controlNets") {
      foreach (ControlList *list, m_controls) {
        foreach (Control *control, *list) {
          m_project->addControl(control);
        }
        delete list;
      }
      m_controls.clear();
    }
    else if (localName == "bundleSettings") {
      // TODO: what to do here????
    }

    return XmlStackedHandler::endElement(namespaceURI, localName, qName);
  }
}
