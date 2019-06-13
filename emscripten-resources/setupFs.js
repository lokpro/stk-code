Module.preRun.push(function() {
  console.log("Prerun:putting user data into memory");
  window.userDataMount = FS.mount(IDBFS, {}, "/home/web_user").mount;
  userDataMount.type.syncfs(true, function(err) {
    if (err) {
      throw err;
    }
    console.log("Read user data into memory");
  });
  window.userDataSync = function userDataSync() {
    function doSync() {
      window.isSyncing = true;
      console.log("Syncing");
      window.userDataMount.type.syncfs(false, function(err) {
        if (err) {
          throw err;
        }
        console.log("Sync'd");
        if (window.needsSync) {
          console.log("More syncing needed");
          doSync();
        } else window.isSyncing = false;
      });
    }

    console.log("Sync requested");

    if (!window.isSyncing) {
      doSync();
    } else {
      window.needsSync = true;
    }
  };
});
