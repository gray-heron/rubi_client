#pragma once
#include <csignal>
#include <cstring>
#include <exception>
#include <string>

class RubiException : public std::exception
{
  protected:
  public:
    std::string rubi_msg;

    const char *what() const throw() { return rubi_msg.c_str(); };

    virtual ~RubiException() throw(){};
    RubiException(std::string what) : rubi_msg(what) {}
};

class CanFailureException : public RubiException
{
  public:
    CanFailureException(std::string msg)
        : RubiException(std::string("Can failure: ") + msg){};
    virtual ~CanFailureException() throw(){};
};
