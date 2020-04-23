This directory contains the version of unrar.dll built without
decryption code, authenticity verification code and RARSetPassword
function. It is intended for use in applictions where presence
of encryption or decryption code is not allowed because of legal
restrictions.

This DLL can be distinguished from usual unrar.dll by missing
RARSetPassword exported function. It sends UCM_NEEDPASSWORD notification,
but returns CRC error for encrypted files regardless of password
provided by application.
