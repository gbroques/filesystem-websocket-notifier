# Library Comparison

* [dmon](https://github.com/septag/dmon) - doesn't have a way to differentiate between files and directories (see [issue #35](https://github.com/septag/dmon/issues/35)).
* [fswatch](https://github.com/emcrisostomo/fswatch) - requires filters for removing noisy inotify `PlatformSpecific` events, and more critically doesn't detect object file creation during recursive monitorying (e.g doesn't detect creation of `.git/objects/56/26abf0f72e58d7a153368ba57db4c673c0e171`). `inotifywait` detects this, but has additional code around `inotify` to do so (see [issue #330](https://github.com/emcrisostomo/fswatch/issues/330)).
* [efsw](https://github.com/SpartanJ/efsw) - doesn't have a way to differentiate between files and directories (see [issue #191](https://github.com/SpartanJ/efsw/issues/191)).
* [Node.js fs.watch](https://nodejs.org/docs/latest/api/fs.html#fswatchfilename-options-listener) - only reports `'rename'` or `'change'` `eventType`s instead of create, delete, and update, and move. It also has caveats, and triggered wrapper projects such as [`node-watch`](https://github.com/yuanchuan/node-watch) and [`chokidar`](https://github.com/paulmillr/chokidar).
