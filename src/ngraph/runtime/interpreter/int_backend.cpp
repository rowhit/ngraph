//*****************************************************************************
// Copyright 2017-2018 Intel Corporation
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
//*****************************************************************************

#include "ngraph/runtime/interpreter/int_backend.hpp"
#include "ngraph/except.hpp"
#include "ngraph/op/convert.hpp"
#include "ngraph/op/select.hpp"
#include "ngraph/op/util/binary_elementwise_comparison.hpp"
#include "ngraph/runtime/backend_manager.hpp"
#include "ngraph/runtime/interpreter/int_executor.hpp"
#include "ngraph/util.hpp"

using namespace std;
using namespace ngraph;

extern "C" const char* get_ngraph_version_string()
{
    return NGRAPH_VERSION;
}

extern "C" runtime::Backend* new_backend(const char* configuration_string)
{
    return new runtime::interpreter::INTBackend();
}

runtime::interpreter::INTBackend::INTBackend()
{
}

runtime::interpreter::INTBackend::INTBackend(const vector<string>& unsupported_op_name_list)
    : m_unsupported_op_name_list{unsupported_op_name_list.begin(), unsupported_op_name_list.end()}
{
}

shared_ptr<runtime::Tensor>
    runtime::interpreter::INTBackend::create_tensor(const element::Type& type, const Shape& shape)
{
    return make_shared<runtime::HostTensor>(type, shape, this);
}

shared_ptr<runtime::Tensor> runtime::interpreter::INTBackend::create_tensor(
    const element::Type& type, const Shape& shape, void* memory_pointer)
{
    return make_shared<runtime::HostTensor>(type, shape, memory_pointer, this);
}

runtime::Handle runtime::interpreter::INTBackend::compile(shared_ptr<Function> function,
                                                          bool enable_performance_collection)
{
    std::unique_ptr<INTExecutor> exec{
        new INTExecutor(this, function, enable_performance_collection)};

    return exec;
}

bool runtime::interpreter::INTBackend::is_supported(const Node& node) const
{
    return m_unsupported_op_name_list.find(node.description()) == m_unsupported_op_name_list.end();
}
