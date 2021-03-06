import React from 'react';
import { ContextMenuItem } from 'components/controls/ContextMenu/ContextMenu';
import { deleteIcon, newResourceIcon } from 'constants/icons';
import { ProjectContext } from 'contexts/ProjectContext';
import { projectApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import { useOpenFlag, useSendApiMessageCallback } from 'utils/hooks';
import { ProjectExplorerItemContext } from '../item.context';
import { DeleteDirectoryRequest, DeleteDirectoryResponse } from 'shared/api.requests';


export const useDeleteDirectoryApiCallbackMessage = (path: string) => {
  const { addPendingDirectoryDeletion } = React.useContext(ProjectContext);

  const deleteDirectory = useSendApiMessageCallback<DeleteDirectoryRequest, DeleteDirectoryResponse>(projectApi.deleteDirectory, (error, response) => {
    if (error) {
      return;
    }

    if (response === DeleteDirectoryResponse.FailedToRecycle) {
      if (window.confirm('Failed to recycle directory, delete it permanently?')) {
        addPendingDirectoryDeletion(path);

        sendApiMessage(projectApi.deleteDirectory, {
          directoryPath: path,
          hardDelete: true,
        } as DeleteDirectoryRequest);
      }
    }
  });

  return React.useCallback(() => {
    addPendingDirectoryDeletion(path);

    deleteDirectory({
      directoryPath: path,
    });
  }, [path, deleteDirectory, addPendingDirectoryDeletion]);
};

export const useDirectoryContextMenu = (path: string, childrenLength: number) => {
  const { setResourceCreatorDir: setAssetCreatorDir, openResourceCreator } = React.useContext(ProjectContext);
  const { disableDirectoryDelete, disableAssetCreate } = React.useContext(ProjectExplorerItemContext);

  const [deleteConfirmationOpen, openDeleteConfirmation, closeDeleteConfirmation] = useOpenFlag(false);
  const deleteDirectory = useDeleteDirectoryApiCallbackMessage(path);

  const handleDirectoryDelete = React.useCallback(() => {
    if (childrenLength > 0) {
      openDeleteConfirmation();
    } else {
      deleteDirectory();
    }
  }, [deleteDirectory, openDeleteConfirmation, childrenLength]);

  const handleCreateResource = React.useCallback(() => {
    setAssetCreatorDir(path);
    openResourceCreator();
  }, [path, setAssetCreatorDir, openResourceCreator]);

  const directoryContextMenuItems: ContextMenuItem[] = React.useMemo(() => {
    return [
      {
        id: 'delete-directory',
        text: 'Delete directory',
        icon: deleteIcon,
        disabled: disableDirectoryDelete,
        onClick: handleDirectoryDelete,
      },
      {
        id: 'new-resource',
        text: 'New resource',
        icon: newResourceIcon,
        disabled: disableAssetCreate,
        onClick: handleCreateResource,
      },
    ];
  }, [handleDirectoryDelete, handleCreateResource, disableDirectoryDelete, disableAssetCreate]);

  return {
    directoryContextMenuItems,
    deleteConfirmationOpen,
    closeDeleteConfirmation,
    deleteDirectory,
  };
};
