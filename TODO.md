TODO LIST
=========

  * [X] Logging infrastructure
    * [X] caslib/logger infrastructure
    * [X] wrapping libxml error functions into caslib/logger
    * [X] wrapping caslib/logger functions into apache logging framework

  * [-] Implement locasberos authentication function
    * [X] use caslib to validate service ticket
    * [ ] emit cookie whenever authentication succeeds
    * [ ] validate cookie
    * [ ] inject cas information into the request (username, attributes etc.)
    * [ ] sanity check to avoid header injection

  * [ ] Documentation
    * [ ] doxygen for caslib API
    * [ ] generate and publish gh-pages automatically
  
  * [X] Remove "+build" and make it optional on mkversion.h