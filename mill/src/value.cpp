#include "value.hpp"
#include <iostream>

mill::Value::Value() : referenceCount(0) <% %>

mill::Value::~Value() = default;

void mill::retain(Value const& value) <%
    ++value.referenceCount;
%>

void mill::release(Value const& value) <%
    if (--value.referenceCount == 0) <%
        delete &value;
    %>
%>

void mill::intrusive_ptr_add_ref(Value const* value) <%
    retain(*value);
%>

void mill::intrusive_ptr_release(Value const* value) <%
    release(*value);
%>
