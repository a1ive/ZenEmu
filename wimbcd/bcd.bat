rd /s /q build
md build
cd build

set bcd=/store bcd

set guidwim={19260817-6666-8888-abcd-000000000000}

bcdedit /createstore bcd
bcdedit %bcd% /create {bootmgr} /d "Wim"
bcdedit %bcd% /set {bootmgr} locale en-us
bcdedit %bcd% /set {bootmgr} timeout 1
bcdedit %bcd% /set {bootmgr} displaybootmenu false

bcdedit %bcd% /create {ramdiskoptions}
bcdedit %bcd% /set {ramdiskoptions} ramdisksdidevice "boot"
bcdedit %bcd% /set {ramdiskoptions} ramdisksdipath \boot\boot.sdi

bcdedit %bcd% /create %guidwim% /d "NT6+ WIM" /application OSLOADER
bcdedit %bcd% /set %guidwim% device ramdisk=[boot]\boot.wim,{ramdiskoptions}
bcdedit %bcd% /set %guidwim% osdevice ramdisk=[boot]\boot.wim,{ramdiskoptions}
bcdedit %bcd% /set %guidwim% winpe true
bcdedit %bcd% /set %guidwim% testsigning true
bcdedit %bcd% /set %guidwim% detecthal true
bcdedit %bcd% /set %guidwim% nointegritychecks true

bcdedit %bcd% /set {bootmgr} displayorder %guidwim% /addlast
bcdedit %bcd% /default %guidwim%

cd ..
