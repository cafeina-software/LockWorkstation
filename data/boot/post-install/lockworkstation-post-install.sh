#!/bin/sh
notify --type progress --group "LockWorkstation" \
    --title "Initial setup" --messageID 'lkws' --progress 1.0 \
    --timeout 10 --onClickApp application/x-vnd.LockWorkstation-Pref \
    "LockWorkstation has been installed. Before running the locking application, please first run the preferences tool to configure it."
