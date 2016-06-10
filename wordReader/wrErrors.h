//
//  wrErrors.h
//  word2vecpp
//
//  Created by Max Fomichev on 29/04/16.
//  Copyright © 2016 Pieci Kvadrati. All rights reserved.
//

#ifndef wrErrors_h
#define wrErrors_h

#include <errno.h>

#include <string>

class errorWR_t {
    std::string m_errStr;
    
public:
    explicit errorWR_t(const std::string &_errStr): m_errStr(_errStr) {;}
    std::string err() const {return m_errStr;}
};

#endif /* wrErrors_h */
