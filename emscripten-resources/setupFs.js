Module.preRun.push(function() {
  console.log("Prerun:putting user data into memory");
  window.userDataMount = FS.mount(IDBFS, {}, "/home/web_user").mount;
  userDataMount.type.syncfs(window.userDataMount, true, function(err) {
    if (err) {
      throw err;
    }
    console.log("Read user data into memory");
  });
  window.userDataSync = function userDataSync() {
    function doSync() {
      window.isSyncing = true;
      console.log("Syncing");
      window.userDataMount.type.syncfs(window.userDataMount, false, function(
        err
      ) {
        if (err) {
          throw err;
        }
        console.log("Sync'd");
        if (window.needsSync) {
          console.log("More syncing needed");
          window.needsSync = false;
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
