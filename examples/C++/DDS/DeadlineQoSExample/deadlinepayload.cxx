// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*! 
 * @file deadlinepayload.cpp
 * This source file contains the definition of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifdef _WIN32
// Remove linker warning LNK4221 on Visual Studio
namespace { char dummy; }
#endif

#include "deadlinepayload.h"

#include <fastcdr/Cdr.h>

#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;

#include <utility>

HelloMsg::HelloMsg()
{
    m_deadlinekey = 0;

}

HelloMsg::~HelloMsg()
{
}

HelloMsg::HelloMsg(const HelloMsg &x)
{
    m_deadlinekey = x.m_deadlinekey;
    m_payload = x.m_payload;
}

HelloMsg::HelloMsg(HelloMsg &&x)
{
    m_deadlinekey = x.m_deadlinekey;
    m_payload = std::move(x.m_payload);
}

HelloMsg& HelloMsg::operator=(const HelloMsg &x)
{
    m_deadlinekey = x.m_deadlinekey;
    m_payload = x.m_payload;
    
    return *this;
}

HelloMsg& HelloMsg::operator=(HelloMsg &&x)
{
    m_deadlinekey = x.m_deadlinekey;
    m_payload = std::move(x.m_payload);
    
    return *this;
}

size_t HelloMsg::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;
            
    current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 256 + 1;


    return current_alignment - initial_alignment;
}

size_t HelloMsg::getCdrSerializedSize(const HelloMsg& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;
            
    current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.payload().size() + 1;


    return current_alignment - initial_alignment;
}

void HelloMsg::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_deadlinekey;

    if(m_payload.length() <= 256)
    scdr << m_payload;
    else
        throw eprosima::fastcdr::exception::BadParamException("payload field exceeds the maximum length");
}

void HelloMsg::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_deadlinekey;
    dcdr >> m_payload;
}

size_t HelloMsg::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
	size_t current_align = current_alignment;
            
     current_align += 2 + eprosima::fastcdr::Cdr::alignment(current_align, 2);
     


    return current_align;
}

bool HelloMsg::isKeyDefined()
{
    return true;
}

void HelloMsg::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
	 scdr << m_deadlinekey;
	  
	 
}