# IBM i Password Verifier (ibmi-pw)
Simple IBM i Password Validation module for npm. Can be used to check whether a provided user name (IBM i user profile)
and password are valid. 
**This module only works on IBM i!**

This module verifies that the user ID and password are correct. If the password is not correct, the incorrect password
count is increased. (The QMAXSIGN system value contains the maximum number of incorrect attempts to sign on.) If the
QMAXSGNACN system value is set to disable the user profile, repeated attempts to validate an incorrect password disable
the user ID. This keeps applications from methodically determining user passwords.

# Installation (works for IBM i only)
```
npm install ibmi-pw
```

# Usage
```javascript
var validator = require('ibmi-pw')
console.log("Is password valid? "+ validator.verifyIbmiPw("usrprf","mypassword"));
```

# Prohibited IBM i user profiles

You cannot validate a password for the following system-supplied user profiles:
- QAUTPROF
- QDLFM
- QMSF
- QSNADS
- QTSTRQS
- QCLUMGT
- QDOC
- QNETSPLF
- QSPL
- QCOLSRV
- QDSNX
- QNFSANON
- QSPLJOB
- QDBSHR
- QFNC
- QNTP
- QSRVAGT
- QDBSHRDO
- QGATE
- QPEX
- QSYS
- QDFTOWN
- QLPAUTO
- QPM400
- QTCP
- QDIRSRV
- QLPINSTALL
- QRJE
- QTFTP

# Doc
Sorry, this is it!
