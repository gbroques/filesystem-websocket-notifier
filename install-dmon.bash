#!/bin/bash
if [ -f "./src/dmon.h" ]; then
  echo "dmon.h already downloaded to src/."
else
  if ! command -v curl 2>&1 >/dev/null; then
      echo "curl could not be found"
      exit 1
  fi
  echo "Downloading dmon.h to src/"
  curl -o ./src/dmon.h https://raw.githubusercontent.com/septag/dmon/a56fdb90e787fa2acecb4a956ec7afd400d1715c/dmon.h
fi

