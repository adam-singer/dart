
class _DirectoryEntryImpl extends _EntryImpl implements DirectoryEntry native "*DirectoryEntry" {

  _DirectoryReaderImpl createReader() native;

  void getDirectory(String path, [Object flags = null, EntryCallback successCallback = null, ErrorCallback errorCallback = null]) native;

  void getFile(String path, [Object flags = null, EntryCallback successCallback = null, ErrorCallback errorCallback = null]) native;

  void removeRecursively(VoidCallback successCallback, [ErrorCallback errorCallback = null]) native;
}