#pragma once

#include <string>
#include "DBResult.h"

//==============DB Interface==============//
namespace chaos
{
    namespace db
    {
        const short MAX_DB_IP = 32;
        const short MAX_DB_USER = 64;
        const short MAX_DB_PWD = 64;
        const short MAX_DB_NAME = 64;
        const short MAX_DB_CHARACTER = 16;


        class DBBase
        {
        public:
            DBBase() {}
            virtual ~DBBase() {}

            virtual bool Connect() = 0;

            virtual void DisConnect() = 0;

            virtual int Query(const std::string& cmd, DBResultBase* result) = 0;

            virtual int Ping() { return 0; }
        };

    }   //namespace db    
}   //namespace chaos