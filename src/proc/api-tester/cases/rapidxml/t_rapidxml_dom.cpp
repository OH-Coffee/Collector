#include <string>

#include "gtest/gtest.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"

namespace {

TEST(RapidXmlDomTest, CountsChildrenAndAttributes)
{
    std::string in_xml = "<root id='1' name='test'><first/><second/></root>";
    rapidxml::xml_document<> xml_document;
    xml_document.parse<rapidxml::parse_default>(&in_xml[0]);

    rapidxml::xml_node<>* root_node = xml_document.first_node("root");
    ASSERT_NE(root_node, nullptr);
    EXPECT_EQ(rapidxml::count_children(root_node), 2U);
    EXPECT_EQ(rapidxml::count_attributes(root_node), 2U);
}

TEST(RapidXmlDomTest, AppendsAndRemovesNodesAndAttributes)
{
    rapidxml::xml_document<> xml_document;
    rapidxml::xml_node<>* root_node = xml_document.allocate_node(rapidxml::node_element, "root");
    rapidxml::xml_node<>* child_node = xml_document.allocate_node(rapidxml::node_element, "child");
    rapidxml::xml_attribute<>* attribute = xml_document.allocate_attribute("id", "1");

    xml_document.append_node(root_node);
    root_node->append_node(child_node);
    root_node->append_attribute(attribute);

    ASSERT_EQ(rapidxml::count_children(root_node), 1U);
    ASSERT_EQ(rapidxml::count_attributes(root_node), 1U);

    root_node->remove_node(child_node);
    root_node->remove_attribute(attribute);

    EXPECT_EQ(rapidxml::count_children(root_node), 0U);
    EXPECT_EQ(rapidxml::count_attributes(root_node), 0U);
}

TEST(RapidXmlDomTest, ClonesSubtreeIntoAnotherDocument)
{
    std::string in_xml = "<root id='1'><child>value</child></root>";
    rapidxml::xml_document<> source_document;
    source_document.parse<rapidxml::parse_default>(&in_xml[0]);

    rapidxml::xml_document<> target_document;
    rapidxml::xml_node<>* cloned_node =
        target_document.clone_node(source_document.first_node("root"));
    target_document.append_node(cloned_node);

    ASSERT_NE(cloned_node, nullptr);
    EXPECT_STREQ(cloned_node->first_attribute("id")->value(), "1");
    EXPECT_STREQ(cloned_node->first_node("child")->value(), "value");
    EXPECT_EQ(rapidxml::count_children(cloned_node), 1U);
}

} // namespace
