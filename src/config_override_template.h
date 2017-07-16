//To override a user configuration from user_config.h:
//Make a user_config_override.h file and write the following:
//#ifdef FOO
//#undef FOO
//#endif
//#define FOO "Bar"

//Example: to override the wifi password write:
//#ifdef PSWD
//#undef PSWD
//#endif
//#define PSWD "abc123"
