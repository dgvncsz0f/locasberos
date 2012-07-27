<!--- -*- coding: utf-8; mode: org; -*- -->

# TODO LIST

  * [X] Logging infrastructure
    * [X] caslib/logger infrastructure
    * [X] wrapping libxml error functions into caslib/logger
    * [X] wrapping caslib/logger functions into apache logging framework

  * [ ] Acceptance tests using real CAS server

  * [ ] Refactor apache/nginx tests:

    * [ ] Identify tests specifics to each module

    * [ ] Use DSL to configure webservers;

  * [-] Implement locasberos authentication function
    * [X] use caslib to validate service ticket
    * [X] CASService: defines reasonable default
    * [ ] /login redirect
    * [ ] emit cookie whenever authentication succeeds
    * [ ] validate cookie
    * [ ] inject cas information into the request (username, attributes etc.)
    * [ ] sanity check to avoid header injection
    * [ ] cookie_serialize needs a portable binary format
    * [ ] cookie_serialize/unserialize signature implementation

  * [ ] Documentation
    * [ ] doxygen for caslib API
    * [ ] generate and publish gh-pages automatically
  
  * [X] Remove "+build" and make it optional on mkversion.h
