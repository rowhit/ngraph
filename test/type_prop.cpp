// ----------------------------------------------------------------------------
// Copyright 2017 Nervana Systems Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// ----------------------------------------------------------------------------

#include "gtest/gtest.h"

#include "ngraph/ngraph.hpp"

#include <memory>
using namespace std;
using namespace ngraph;

void test_binary_bad_arguments_tuple(const shared_ptr<Node>& node);
void test_binary_bad_arguments_views(const shared_ptr<Node>& node);
void test_binary_good_arguments(const shared_ptr<Node>& node);
void test_binary(shared_ptr<Node>(f)(const shared_ptr<Node>& x, const shared_ptr<Node>& y));

//
// Tests for broadcast.
//
TEST(type_prop, broadcast_deduce)
{
    // Deduce type
    auto param = make_shared<op::Parameter>(element::Float32::element_type(), Shape{2, 4});
    auto bc    = make_shared<op::Broadcast>(param, Shape{2, 3, 4}, AxisSet{1});
    bc->propagate_types();
    auto bc_vt = bc->get_value_type();
    ASSERT_EQ(*bc_vt, TensorViewType(element::Float32::element_type(), Shape{2, 3, 4}));
}

TEST(type_prop, broadcast_deduce_correct)
{
    // Check deduced type against correctly specified type
    auto param = make_shared<op::Parameter>(element::Float32::element_type(), Shape{2, 4});
    auto bc    = make_shared<op::Broadcast>(param, Shape{2, 3, 4}, AxisSet{1});
    bc->set_value_type(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{2, 3, 4}));
    bc->propagate_types();
    auto bc_vt = bc->get_value_type();
    ASSERT_EQ(*bc_vt, TensorViewType(element::Float32::element_type(), Shape{2, 3, 4}));
}

TEST(type_prop, broadcast_deduce_incorrect)
{
    // Check deduced type against incorrectly specified type
    auto param = make_shared<op::Parameter>(element::Float32::element_type(), Shape{2, 4});
    auto bc    = make_shared<op::Broadcast>(param, Shape{2, 4, 3}, AxisSet{1});
    bc->set_value_type(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{2, 3, 4}));
    try
    {
        bc->propagate_types();
        // Should have thrown, so fail if it didn't
        FAIL() << "Deduced type should disagree with specified type";
    }
    catch (const ngraph_error& error)
    {
        EXPECT_EQ(error.what(), std::string("Broadcast arg, shape, and axes are incompatible"));
    }
    catch (...)
    {
        FAIL() << "Deduced type check failed for unexpected reason";
    }
}

TEST(type_prop, broadcast_bad_arguments)
{
    // Check for bad arguments
    auto param = make_shared<op::Parameter>(make_shared<TupleType>());
    auto bc    = make_shared<op::Broadcast>(param, Shape{2, 4, 3}, AxisSet{1});
    try
    {
        bc->propagate_types();
        // Should have thrown, so fail if it didn't
        FAIL() << "Tuple argument to broadcast not detected.";
    }
    catch (const ngraph_error& error)
    {
        EXPECT_EQ(error.what(), std::string("Argument to broadcast is not a tensor view"));
    }
    catch (...)
    {
        FAIL() << "Deduced type check failed for unexpected reason";
    }
}

//
// Tests for dot product.
//
TEST(type_prop, dot_deduce_scalar_2d)
{
    // Deduce type for 1D arguments
    auto param1 = make_shared<op::Parameter>(element::Float32::element_type(), Shape{});
    auto param2 = make_shared<op::Parameter>(element::Float32::element_type(), Shape{4,5});
    auto bc     = make_shared<op::Dot>(param1, param2);
    bc->propagate_types();
    auto bc_vt = bc->get_value_type();
    ASSERT_EQ(*bc_vt, TensorViewType(element::Float32::element_type(), Shape{4,5}));
}

TEST(type_prop, dot_deduce_2d_scalar)
{
    // Deduce type for 1D arguments
    auto param1 = make_shared<op::Parameter>(element::Float32::element_type(), Shape{4,5});
    auto param2 = make_shared<op::Parameter>(element::Float32::element_type(), Shape{});
    auto bc     = make_shared<op::Dot>(param1, param2);
    bc->propagate_types();
    auto bc_vt = bc->get_value_type();
    ASSERT_EQ(*bc_vt, TensorViewType(element::Float32::element_type(), Shape{4,5}));
}

TEST(type_prop, dot_deduce_scalar_scalar)
{
    // Deduce type for 1D arguments
    auto param1 = make_shared<op::Parameter>(element::Float32::element_type(), Shape{});
    auto param2 = make_shared<op::Parameter>(element::Float32::element_type(), Shape{});
    auto bc     = make_shared<op::Dot>(param1, param2);
    bc->propagate_types();
    auto bc_vt = bc->get_value_type();
    ASSERT_EQ(*bc_vt, TensorViewType(element::Float32::element_type(), Shape{}));
}

TEST(type_prop, dot_deduce_scalar_1d)
{
    // Deduce type for 1D arguments
    auto param1 = make_shared<op::Parameter>(element::Float32::element_type(), Shape{});
    auto param2 = make_shared<op::Parameter>(element::Float32::element_type(), Shape{6});
    auto bc     = make_shared<op::Dot>(param1, param2);
    bc->propagate_types();
    auto bc_vt = bc->get_value_type();
    ASSERT_EQ(*bc_vt, TensorViewType(element::Float32::element_type(), Shape{6}));
}

TEST(type_prop, dot_deduce_1d)
{
    // Deduce type for 1D arguments
    auto param1 = make_shared<op::Parameter>(element::Float32::element_type(), Shape{4});
    auto param2 = make_shared<op::Parameter>(element::Float32::element_type(), Shape{4});
    auto bc     = make_shared<op::Dot>(param1, param2);
    bc->propagate_types();
    auto bc_vt = bc->get_value_type();
    ASSERT_EQ(*bc_vt, TensorViewType(element::Float32::element_type(), Shape{}));
}

TEST(type_prop, dot_deduce_2d)
{
    // Deduce type for 2D arguments
    auto param1 = make_shared<op::Parameter>(element::Float32::element_type(), Shape{4,2});
    auto param2 = make_shared<op::Parameter>(element::Float32::element_type(), Shape{2,3});
    auto bc     = make_shared<op::Dot>(param1, param2);
    bc->propagate_types();
    auto bc_vt = bc->get_value_type();
    ASSERT_EQ(*bc_vt, TensorViewType(element::Float32::element_type(), Shape{4,3}));
}

TEST(type_prop, dot_deduce_different_d)
{
    // Deduce type for different-dimension arguments
    auto param1 = make_shared<op::Parameter>(element::Float32::element_type(), Shape{2,8,4,2});
    auto param2 = make_shared<op::Parameter>(element::Float32::element_type(), Shape{1,2,3});
    auto bc     = make_shared<op::Dot>(param1, param2);
    bc->propagate_types();
    auto bc_vt = bc->get_value_type();
    ASSERT_EQ(*bc_vt, TensorViewType(element::Float32::element_type(), Shape{2,8,4,1,3}));
}

TEST(type_prop, dot_deduce_different_d_correct)
{
    // Deduced type matches explicitly set type
    auto param1 = make_shared<op::Parameter>(element::Float32::element_type(), Shape{2,8,4,2});
    auto param2 = make_shared<op::Parameter>(element::Float32::element_type(), Shape{1,2,3});
    auto bc     = make_shared<op::Dot>(param1, param2);
    bc->set_value_type(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{2,8,4,1,3}));
    bc->propagate_types();
    auto bc_vt = bc->get_value_type();
    ASSERT_EQ(*bc_vt, TensorViewType(element::Float32::element_type(), Shape{2,8,4,1,3}));
}

TEST(type_prop, dot_deduce_element_type_mismatch)
{
    // Type deduction fails due to element type mismatch
    auto param1 = make_shared<op::Parameter>(element::Float32::element_type(), Shape{4,2});
    auto param2 = make_shared<op::Parameter>(element::Int32::element_type(), Shape{2,5});
    auto bc     = make_shared<op::Dot>(param1, param2);
    try
    {
        bc->propagate_types();
        // Should have thrown, so fail if it didn't
        FAIL() << "Element type mismatch not detected";
    }
    catch (const ngraph_error& error)
    {
        EXPECT_EQ(error.what(), std::string("Arguments to dot must have the same element type"));
    }
    catch (...)
    {
        FAIL() << "Deduced type check failed for unexpected reason";
    }
}

TEST(type_prop, dot_deduce_reduction_axes_size_mismatch)
{
    // Type deduction fails due to reduction axes size mismatch
    auto param1 = make_shared<op::Parameter>(element::Float32::element_type(), Shape{4,2});
    auto param2 = make_shared<op::Parameter>(element::Float32::element_type(), Shape{3,5});
    auto bc     = make_shared<op::Dot>(param1, param2);
    try
    {
        bc->propagate_types();
        // Should have thrown, so fail if it didn't
        FAIL() << "Dot reduction axes size mismatch not detected";
    }
    catch (const ngraph_error& error)
    {
        EXPECT_EQ(error.what(), std::string("Dot reduction axes not compatible"));
    }
    catch (...)
    {
        FAIL() << "Deduced type check failed for unexpected reason";
    }
}

//
// Tests for binary elementwise ops.
//
void test_binary_bad_arguments_tuple(const shared_ptr<Node>& node)
{
    try
    {
        node->propagate_types();
        // Should have thrown, so fail if it didn't
        FAIL() << "Tuple argument not detected.";
    }
    catch (const ngraph_error& error)
    {
        EXPECT_EQ(error.what(), std::string("Arguments must be tensor views"));
    }
    catch (...)
    {
        FAIL() << "Deduced type check failed for unexpected reason";
    }
}

void test_binary_bad_arguments_view_shapes(const shared_ptr<Node>& node)
{
    try
    {
        node->propagate_types();
        // Should have thrown, so fail if it didn't
        FAIL() << "Incompatible view arguments not detected.";
    }
    catch (const ngraph_error& error)
    {
        EXPECT_EQ(error.what(), std::string("Arguments must have the same tensor view shape"));
    }
    catch (...)
    {
        FAIL() << "Deduced type check failed for unexpected reason";
    }
}

void test_binary_bad_arguments_view_element_types(const shared_ptr<Node>& node)
{
    try
    {
        node->propagate_types();
        // Should have thrown, so fail if it didn't
        FAIL() << "Incompatible view arguments not detected.";
    }
    catch (const ngraph_error& error)
    {
        EXPECT_EQ(error.what(), std::string("Arguments must have the same tensor view element type"));
    }
    catch (...)
    {
        FAIL() << "Deduced type check failed for unexpected reason";
    }
}

void test_binary_good_arguments(const shared_ptr<Node>& node)
{
    node->propagate_types();
    EXPECT_EQ(*node->get_value_type(), *node->get_arguments()[0]->get_value_type());
}

void test_binary(shared_ptr<Node>(f)(const shared_ptr<Node>& x, const shared_ptr<Node>& y))
{
    // Check for bad arguments
    auto tp0_param       = make_shared<op::Parameter>(make_shared<TupleType>());
    auto tp1_param       = make_shared<op::Parameter>(make_shared<TupleType>());
    auto tv0_2_4_param_0 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{2, 4}));
    auto tv0_2_4_param_1 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{2, 4}));
    auto tv0_2_4_param_2 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Int32::element_type(), Shape{2, 4}));
    auto tv0_4_2_param = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{4, 2}));

    test_binary_bad_arguments_tuple(f(tp0_param, tp1_param));
    test_binary_bad_arguments_tuple(f(tp0_param, tv0_2_4_param_0));
    test_binary_bad_arguments_tuple(f(tv0_2_4_param_0, tp0_param));
    test_binary_bad_arguments_view_shapes(f(tv0_2_4_param_0, tv0_4_2_param));
    test_binary_bad_arguments_view_element_types(f(tv0_2_4_param_0, tv0_2_4_param_2));
    test_binary_good_arguments(f(tv0_2_4_param_0, tv0_2_4_param_1));
}

TEST(type_prop, add_bad_arguments)
{
    test_binary([](const shared_ptr<Node>& x, const shared_ptr<Node>& y) -> shared_ptr<Node> {
        return make_shared<op::Add>(x, y);
    });
}

TEST(type_prop, divide_bad_arguments)
{
    test_binary([](const shared_ptr<Node>& x, const shared_ptr<Node>& y) -> shared_ptr<Node> {
        return make_shared<op::Divide>(x, y);
    });
}

TEST(type_prop, multiply_bad_arguments)
{
    test_binary([](const shared_ptr<Node>& x, const shared_ptr<Node>& y) -> shared_ptr<Node> {
        return make_shared<op::Multiply>(x, y);
    });
}

TEST(type_prop, subtract_bad_arguments)
{
    test_binary([](const shared_ptr<Node>& x, const shared_ptr<Node>& y) -> shared_ptr<Node> {
        return make_shared<op::Subtract>(x, y);
    });
}

TEST(type_prop, comparison_good)
{
    auto tv0_2_4_param_0 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{2, 4}));
    auto tv0_2_4_param_1 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{2, 4}));
    auto eq = make_shared<op::Equal>(tv0_2_4_param_0,tv0_2_4_param_1);
    TensorViewType expected_type{element::Bool::element_type(), Shape{2, 4}};
    eq->propagate_types();
    EXPECT_EQ(*eq->get_value_type(),expected_type);
}

TEST(type_prop, binary_arithmetic_bad_argument_element_types)
{
    auto tv0_2_4_param_0 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Bool::element_type(), Shape{2, 4}));
    auto tv0_2_4_param_1 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Bool::element_type(), Shape{2, 4}));
    auto bc = make_shared<op::Add>(tv0_2_4_param_0,tv0_2_4_param_1);
    try
    {
        bc->propagate_types();
        // Should have thrown, so fail if it didn't
        FAIL() << "Did not detect incorrect element types for arithmetic operator";
    }
    catch (const ngraph_error& error)
    {
        EXPECT_EQ(error.what(), std::string("Operands for arithmetic operators must have numeric element type"));
    }
    catch (...)
    {
        FAIL() << "Deduced type check failed for unexpected reason";
    }
}

TEST(type_prop, unary_arithmetic_bad_argument_element_types)
{
    auto tv0_2_4_param = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Bool::element_type(), Shape{2, 4}));
    auto bc = make_shared<op::Negative>(tv0_2_4_param);
    try
    {
        bc->propagate_types();
        // Should have thrown, so fail if it didn't
        FAIL() << "Did not detect incorrect element types for arithmetic operator";
    }
    catch (const ngraph_error& error)
    {
        EXPECT_EQ(error.what(), std::string("Operands for arithmetic operators must have numeric element type"));
    }
    catch (...)
    {
        FAIL() << "Deduced type check failed for unexpected reason";
    }
}

TEST(type_prop, select_deduce)
{
    auto tv0_2_4_param_0 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Bool::element_type(), Shape{2, 4}));
    auto tv0_2_4_param_1 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{2, 4}));
    auto tv0_2_4_param_2 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{2, 4}));
    auto bc = make_shared<op::Select>(tv0_2_4_param_0,tv0_2_4_param_1,tv0_2_4_param_2);
    bc->propagate_types();
    auto bc_vt = bc->get_value_type();
    ASSERT_EQ(*bc_vt, TensorViewType(element::Float32::element_type(), Shape{2, 4}));
}

TEST(type_prop, select_deduce_correct)
{
    auto tv0_2_4_param_0 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Bool::element_type(), Shape{2, 4}));
    auto tv0_2_4_param_1 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{2, 4}));
    auto tv0_2_4_param_2 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{2, 4}));
    auto bc = make_shared<op::Select>(tv0_2_4_param_0,tv0_2_4_param_1,tv0_2_4_param_2);
    bc->set_value_type(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{2, 4}));
    bc->propagate_types();
    auto bc_vt = bc->get_value_type();
    ASSERT_EQ(*bc_vt, TensorViewType(element::Float32::element_type(), Shape{2, 4}));
}

TEST(type_prop, select_shape_mismatch_a)
{
    auto tv0_2_4_param_0 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Bool::element_type(), Shape{3, 5}));
    auto tv0_2_4_param_1 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{2, 4}));
    auto tv0_2_4_param_2 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{2, 4}));
    auto bc = make_shared<op::Select>(tv0_2_4_param_0,tv0_2_4_param_1,tv0_2_4_param_2);
    try
    {
        bc->propagate_types();
        // Should have thrown, so fail if it didn't
        FAIL() << "Did not detect incorrect element types for arithmetic operator";
    }
    catch (const ngraph_error& error)
    {
        EXPECT_EQ(error.what(), std::string("Arguments must have the same tensor view shape"));
    }
    catch (...)
    {
        FAIL() << "Deduced type check failed for unexpected reason";
    }
}

TEST(type_prop, select_shape_mismatch_b)
{
    auto tv0_2_4_param_0 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Bool::element_type(), Shape{2, 4}));
    auto tv0_2_4_param_1 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{3, 5}));
    auto tv0_2_4_param_2 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{2, 4}));
    auto bc = make_shared<op::Select>(tv0_2_4_param_0,tv0_2_4_param_1,tv0_2_4_param_2);
    try
    {
        bc->propagate_types();
        // Should have thrown, so fail if it didn't
        FAIL() << "Did not detect incorrect element types for arithmetic operator";
    }
    catch (const ngraph_error& error)
    {
        EXPECT_EQ(error.what(), std::string("Arguments must have the same tensor view shape"));
    }
    catch (...)
    {
        FAIL() << "Deduced type check failed for unexpected reason";
    }
}

TEST(type_prop, select_shape_mismatch_c)
{
    auto tv0_2_4_param_0 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Bool::element_type(), Shape{2, 4}));
    auto tv0_2_4_param_1 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{2, 4}));
    auto tv0_2_4_param_2 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{3, 5}));
    auto bc = make_shared<op::Select>(tv0_2_4_param_0,tv0_2_4_param_1,tv0_2_4_param_2);
    try
    {
        bc->propagate_types();
        // Should have thrown, so fail if it didn't
        FAIL() << "Did not detect incorrect element types for arithmetic operator";
    }
    catch (const ngraph_error& error)
    {
        EXPECT_EQ(error.what(), std::string("Arguments must have the same tensor view shape"));
    }
    catch (...)
    {
        FAIL() << "Deduced type check failed for unexpected reason";
    }
}

TEST(type_prop, select_elem_mismatch_a)
{
    auto tv0_2_4_param_0 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{2, 4}));
    auto tv0_2_4_param_1 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{2, 4}));
    auto tv0_2_4_param_2 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{2, 4}));
    auto bc = make_shared<op::Select>(tv0_2_4_param_0,tv0_2_4_param_1,tv0_2_4_param_2);
    try
    {
        bc->propagate_types();
        // Should have thrown, so fail if it didn't
        FAIL() << "Did not detect incorrect element types for arithmetic operator";
    }
    catch (const ngraph_error& error)
    {
        EXPECT_EQ(error.what(), std::string("Argument 0 for arithmetic operators must have boolean element type"));
    }
    catch (...)
    {
        FAIL() << "Deduced type check failed for unexpected reason";
    }
}

TEST(type_prop, select_elem_mismatch_bc)
{
    auto tv0_2_4_param_0 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Bool::element_type(), Shape{2, 4}));
    auto tv0_2_4_param_1 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Float32::element_type(), Shape{2, 4}));
    auto tv0_2_4_param_2 = make_shared<op::Parameter>(
        make_shared<TensorViewType>(element::Int32::element_type(), Shape{2, 4}));
    auto bc = make_shared<op::Select>(tv0_2_4_param_0,tv0_2_4_param_1,tv0_2_4_param_2);
    try
    {
        bc->propagate_types();
        // Should have thrown, so fail if it didn't
        FAIL() << "Did not detect incorrect element types for arithmetic operator";
    }
    catch (const ngraph_error& error)
    {
        EXPECT_EQ(error.what(), std::string("Arguments 1 and 2 must have the same tensor view type"));
    }
    catch (...)
    {
        FAIL() << "Deduced type check failed for unexpected reason";
    }
}

