/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QMouseEvent>
#include <QMessageBox>
#include <QHeaderView>
#include <QScrollBar>

// CTK includes
#include "ctkModelTester.h"

// qMRML includes
#include "qMRMLSceneModel.h"
//#include "qMRMLIGTLIOSortFilterProxyModel.h"
#include "qMRMLSortFilterProxyModel.h"
#include "qMRMLSceneTransformModel.h"
#include "qMRMLIGTLIOModel.h"
#include "qMRMLTreeView.h"
#include "qMRMLItemDelegate.h"

// OpenIGTLinkIF GUI includes
#include "qMRMLIGTLIOTreeView.h"

// OpenIGTLinkIF Logic includes
#include "vtkSlicerOpenIGTLinkIFLogic.h"

// MRML includes
#include <vtkMRMLNode.h>
#include "vtkMRMLIGTLConnectorNode.h"

//------------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_OpenIGTLinkIF
class qMRMLIGTLIOTreeViewPrivate
{
  Q_DECLARE_PUBLIC(qMRMLIGTLIOTreeView);
protected:
  qMRMLIGTLIOTreeView* const q_ptr;
public:
  qMRMLIGTLIOTreeViewPrivate(qMRMLIGTLIOTreeView& object);
  void init();

  //void setSortFilterProxyModel(qMRMLIGTLIOSortFilterProxyModel* newSortModel);
  void setSortFilterProxyModel(qMRMLSortFilterProxyModel* newSortModel);

  qMRMLIGTLIOModel*                 SceneModel;
  //qMRMLIGTLIOSortFilterProxyModel*        SortFilterModel;
  qMRMLSortFilterProxyModel*        SortFilterModel;
  vtkSlicerOpenIGTLinkIFLogic*      Logic;
};

//------------------------------------------------------------------------------
qMRMLIGTLIOTreeViewPrivate::qMRMLIGTLIOTreeViewPrivate(qMRMLIGTLIOTreeView& object)
  : q_ptr(&object)
{
  this->SceneModel = 0;
  this->SortFilterModel = 0;
}

//------------------------------------------------------------------------------
void qMRMLIGTLIOTreeViewPrivate::init()
{
  Q_Q(qMRMLIGTLIOTreeView);

  q->setItemDelegate(new qMRMLItemDelegate(q));

  QObject::connect(q, SIGNAL(clicked(QModelIndex)),
                   q, SLOT(onClicked(QModelIndex)));

  ////qMRMLIGTLIOSortFilterProxyModel* pmodel = new qMRMLIGTLIOSortFilterProxyModel(q);
  //qMRMLSortFilterProxyModel* pmodel = new qMRMLSortFilterProxyModel(q);
  //this->setSortFilterProxyModel(pmodel);
  //
  //q->setSceneModelType("IGTLConnector");
  //
  //q->setUniformRowHeights(true);
  
  //QObject::connect(q, SIGNAL(collapsed(QModelIndex)),
  //                 q, SLOT(onNumberOfVisibleIndexChanged()));
  //QObject::connect(q, SIGNAL(expanded(QModelIndex)),
  //                 q, SLOT(onNumberOfVisibleIndexChanged()));
  //q->horizontalScrollBar()->installEventFilter(q);
  //
  //this->NodeMenu = new QMenu(q);
  //
  //// rename node
  //QAction* renameAction =
  //  new QAction(qMRMLTreeView::tr("Rename"),this->NodeMenu);
  //this->NodeMenu->addAction(renameAction);
  //QObject::connect(renameAction, SIGNAL(triggered()),
  //                 q, SLOT(renameCurrentNode()));
  //
  //// delete node
  //QAction* deleteAction =
  //  new QAction(qMRMLTreeView::tr("Delete"),this->NodeMenu);
  //this->NodeMenu->addAction(deleteAction);
  //QObject::connect(deleteAction, SIGNAL(triggered()),
  //                 q, SLOT(deleteCurrentNode()));
  //// EditAction is hidden by default
  //this->EditAction =
  //  new QAction(qMRMLTreeView::tr("Edit properties..."), this->NodeMenu);
  //QObject::connect(this->EditAction, SIGNAL(triggered()),
  //                 q, SLOT(editCurrentNode()));
  //this->SceneMenu = new QMenu(q);

  //qMRMLSortFilterProxyModel* pmodel = new qMRMLSortFilterProxyModel(q);
  //this->setSortFilterProxyModel(pmodel);
  //q->setSceneModelType("IGTLConnector");

  // Working version
  this->SceneModel = new qMRMLIGTLIOModel(q);
  q->setSceneModel(this->SceneModel, "IGTLConnector");
  //this->SortFilterModel = new qMRMLSortFilterProxyModel(q);
  // we only want to show vtkMRMLAnnotationNodes and vtkMRMLAnnotationHierarchyNodes
  QStringList nodeTypes = QStringList();
  nodeTypes.append("vtkMRMLIGTLConnectorNode");
  
  q->setNodeTypes(nodeTypes);
  //this->SortFilterModel = q->sortFilterProxyModel();

  q->setUniformRowHeights(true);
  this->SortFilterModel = q->sortFilterProxyModel();

  q->expandAll();

}


//------------------------------------------------------------------------------
//void qMRMLIGTLIOTreeViewPrivate::setSortFilterProxyModel(qMRMLIGTLIOSortFilterProxyModel* newSortModel)
void qMRMLIGTLIOTreeViewPrivate::setSortFilterProxyModel(qMRMLSortFilterProxyModel* newSortModel)
{
  Q_Q(qMRMLIGTLIOTreeView);

  if (newSortModel == this->SortFilterModel)
    {
    return;
    }
  
  // delete the previous filter
  delete this->SortFilterModel;
  this->SortFilterModel = newSortModel;
  // Set the input of the view
  // if no filter is given then let's show the scene model directly
  q->QTreeView::setModel(this->SortFilterModel
    ? static_cast<QAbstractItemModel*>(this->SortFilterModel)
    : static_cast<QAbstractItemModel*>(this->SceneModel));
  // Setting a new model to the view resets the selection model
  QObject::connect(q->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                   q, SLOT(onCurrentRowChanged(QModelIndex)));
  if (!this->SortFilterModel)
    {
    return;
    }
  this->SortFilterModel->setParent(q);
  // Set the input of the filter
  this->SortFilterModel->setSourceModel(this->SceneModel);

  // resize the view if new rows are added/removed
  QObject::connect(this->SortFilterModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                   q, SLOT(onNumberOfVisibleIndexChanged()));
  QObject::connect(this->SortFilterModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                   q, SLOT(onNumberOfVisibleIndexChanged()));

  q->expandAll();
  //q->expandToDepth(2);
  //q->onNumberOfVisibleIndexChanged();
}



//------------------------------------------------------------------------------
qMRMLIGTLIOTreeView::qMRMLIGTLIOTreeView(QWidget *_parent)
  : Superclass(_parent)
  , d_ptr(new qMRMLIGTLIOTreeViewPrivate(*this))
    //  : d_ptr(new qMRMLIGTLIOTreeViewPrivate(*this))
{
  Q_D(qMRMLIGTLIOTreeView);
  d->init();

  // we need to enable mouse tracking to set the appropriate cursor while mouseMove occurs
  this->setMouseTracking(true);
}

//------------------------------------------------------------------------------
qMRMLIGTLIOTreeView::~qMRMLIGTLIOTreeView()
{
}

//------------------------------------------------------------------------------
void qMRMLIGTLIOTreeView::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLIGTLIOTreeView);
  Q_ASSERT(d->SortFilterModel);
  // only qMRMLSceneModel needs the scene, the other proxies don't care.
  d->SceneModel->setMRMLScene(scene);

  this->expandAll();
}

//------------------------------------------------------------------------------
// Click and selected event handling
//------------------------------------------------------------------------------

int qMRMLIGTLIOTreeView::rowProperty(const QModelIndex& index, vtkMRMLIGTLConnectorNode* &cnode, int & dir)
{
  Q_D(qMRMLIGTLIOTreeView);
  Q_ASSERT(d->SortFilterModel);

  //vtkMRMLIGTLConnectorNode * cnode;
  QStandardItem* parent1;
  QStandardItem* grandParent;

  cnode = NULL;
  dir = vtkMRMLIGTLConnectorNode::IO_UNSPECIFIED;
  
  qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(d->SortFilterModel->sourceModel());
  QStandardItem* item = sceneModel->itemFromIndex(d->SortFilterModel->mapToSource(index));
  if (!item)
    {
    return TYPE_UNKNOWN;
    }
  
  parent1 = item->parent();
  if (!parent1)
    {
    return TYPE_ROOT; // Root
    }

  // Check if the item is "IN" or "OUT", parent must be connector node
  QString text = parent1->child(item->row(), 0)->text();
  if (text.compare(QString("IN")) == 0)
    {
    vtkMRMLNode* node = sceneModel->mrmlNodeFromIndex(parent1->index());
    cnode = vtkMRMLIGTLConnectorNode::SafeDownCast(node);
    dir = vtkMRMLIGTLConnectorNode::IO_INCOMING;
    return TYPE_STREAM;
    }
  else if (text.compare(QString("OUT")) == 0)
    {
    vtkMRMLNode* node = sceneModel->mrmlNodeFromIndex(parent1->index());
    cnode = vtkMRMLIGTLConnectorNode::SafeDownCast(node);
    dir = vtkMRMLIGTLConnectorNode::IO_OUTGOING;
    return TYPE_STREAM;
    }

  // If not, the selected item must be a data node
  grandParent = parent1->parent();
  if (!grandParent)
    {
    return TYPE_UNKNOWN;
    }

  vtkMRMLNode* node = sceneModel->mrmlNodeFromIndex(grandParent->index());
  cnode = vtkMRMLIGTLConnectorNode::SafeDownCast(node);
  if (cnode && parent1 && grandParent)
    {
    QString text = grandParent->child(parent1->row(), 0)->text();
    if (text.compare(QString("IN")) == 0)
      {
      dir = vtkMRMLIGTLConnectorNode::IO_INCOMING;
      return TYPE_DATANODE;
      }
    else
      {
      dir = vtkMRMLIGTLConnectorNode::IO_OUTGOING;
      return TYPE_DATANODE;
      }
    }
  else
    {
    cnode = NULL;
    dir = vtkMRMLIGTLConnectorNode::IO_UNSPECIFIED;
    return TYPE_UNKNOWN;
    }
}


void qMRMLIGTLIOTreeView::onClicked(const QModelIndex& index)
{
  Q_D(qMRMLIGTLIOTreeView);

  if (index.column() == qMRMLIGTLIOModel::VisualizationColumn)
    {
    //this->onVisualizationColumnClicked(mrmlNode);
    }
}


void qMRMLIGTLIOTreeView::onCurrentRowChanged(const QModelIndex& index)
{
  Q_D(qMRMLIGTLIOTreeView);
  Q_ASSERT(d->SortFilterModel);
  //Q_ASSERT(this->currentNode() == d->SortFilterModel->mrmlNodeFromIndex(index));

  vtkMRMLIGTLConnectorNode* cnode;
  int dir;
  int type = rowProperty(index, cnode, dir);

  //emit connectorNodeUpdated(cnode, dir);
  emit ioTreeViewUpdated(type, cnode, dir);

}



//------------------------------------------------------------------------------
//
// MouseMove event handling
//
//------------------------------------------------------------------------------

#ifndef QT_NO_CURSOR
//------------------------------------------------------------------------------
bool qMRMLIGTLIOTreeView::viewportEvent(QEvent* e)
{

  // reset the cursor if we leave the viewport
  if(e->type() == QEvent::Leave)
    {
    setCursor(QCursor());
    }

  return QTreeView::viewportEvent(e);
}

//------------------------------------------------------------------------------
void qMRMLIGTLIOTreeView::mouseMoveEvent(QMouseEvent* e)
{
  this->QTreeView::mouseMoveEvent(e);

  // get the index of the current column
  QModelIndex index = indexAt(e->pos());

  //if (index.column() == qMRMLSceneIGTLConnectorModel::VisibilityColumn || index.column() == qMRMLSceneIGTLConnectorModel::LockColumn || index.column() == qMRMLSceneIGTLConnectorModel::EditColumn)
  //  {
  //  // we are over a column with a clickable icon
  //  // let's change the cursor
  //  QCursor handCursor(Qt::PointingHandCursor);
  //  this->setCursor(handCursor);
  //  // .. and bail out
  //  return;
  //  }
  //else if(this->cursor().shape() == Qt::PointingHandCursor)
  //  {
  //  // if we are NOT over such a column and we already have a changed cursor,
  //  // reset it!
  //  this->setCursor(QCursor());
  //  }

}
#endif

//------------------------------------------------------------------------------
//
// Layout and behavior customization
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void qMRMLIGTLIOTreeView::setSelectedNode(const char* id)
{
  Q_D(qMRMLIGTLIOTreeView);

  vtkMRMLNode* node = this->mrmlScene()->GetNodeByID(id);

  if (node)
    {
    this->setCurrentIndex(d->SortFilterModel->indexFromMRMLNode(node));
    //this->setCurrentIndex(d->SceneModel->indexFromNode(node));
    }
}

//------------------------------------------------------------------------------
//
// In-Place Editing of Annotations
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//void qMRMLIGTLIOTreeView::onVisibilityColumnClicked(vtkMRMLNode* node)
//{
//  if (!node)
//    {
//    // no node found!
//    return;
//    }

//  //vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);

//  //if (annotationNode)
//  //  {
//  //  // this is a valid annotationNode
//  //  annotationNode->SetVisible(!annotationNode->GetVisible());
//  //
//  //  }


//  // TODO move to logic
//  //vtkMRMLAnnotationHierarchyNode* hierarchyNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(node);
//  //
//  //if (hierarchyNode)
//  //  {
//  //  vtkCollection* children = vtkCollection::New();
//  //  hierarchyNode->GetChildrenDisplayableNodes(children);
//  //
//  //  children->InitTraversal();
//  //  for (int i=0; i<children->GetNumberOfItems(); ++i)
//  //    {
//  //    vtkMRMLAnnotationNode* childNode = vtkMRMLAnnotationNode::SafeDownCast(children->GetItemAsObject(i));
//  //    if (childNode)
//  //      {
//  //      // this is a valid annotation child node
//  //      //
//  //      childNode->SetVisible(!childNode->GetVisible());
//  //      }
//  //    } // for loop
//  //
//  //  } // if hierarchyNode
//}

//------------------------------------------------------------------------------
//void qMRMLIGTLIOTreeView::onLockColumnClicked(vtkMRMLNode* node)
//{

//  if (!node)
//    {
//    // no node found!
//    return;
//    }

//  //vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
//  //
//  //if (annotationNode)
//  //  {
//  //  // this is a valid annotationNode
//  //  annotationNode->SetLocked(!annotationNode->GetLocked());
//  //
//  //  }


//  // TODO move to logic
//  //vtkMRMLAnnotationHierarchyNode* hierarchyNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(node);
//  //
//  //if (hierarchyNode)
//  //  {
//  //  vtkCollection* children = vtkCollection::New();
//  //  hierarchyNode->GetChildrenDisplayableNodes(children);
//  //
//  //  children->InitTraversal();
//  //  for (int i=0; i<children->GetNumberOfItems(); ++i)
//  //    {
//  //    vtkMRMLAnnotationNode* childNode = vtkMRMLAnnotationNode::SafeDownCast(children->GetItemAsObject(i));
//  //    if (childNode)
//  //      {
//  //      // this is a valid annotation child node
//  //      //
//  //      childNode->SetLocked(!childNode->GetLocked());
//  //      }
//  //    } // for loop
//  //
//  //  } // if hierarchyNode
//}

//------------------------------------------------------------------------------
void qMRMLIGTLIOTreeView::mousePressEvent(QMouseEvent* event)
{
  // skip qMRMLTreeView
  this->QTreeView::mousePressEvent(event);
}

//------------------------------------------------------------------------------
//
// Connections to other classes
//
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Set and observe the logic
//-----------------------------------------------------------------------------
void qMRMLIGTLIOTreeView::setLogic(vtkSlicerOpenIGTLinkIFLogic* logic)
{
  Q_D(qMRMLIGTLIOTreeView);
  if (!logic)
    {
    return;
    }

  d->Logic = logic;

  // propagate down to model
  d->SceneModel->setLogic(d->Logic);
}