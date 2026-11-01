#include <string>

#include "gtest/gtest.h"
#include "rapidxml/rapidxml.hpp"

namespace {

TEST(RapidXmlParseTest, ParsesNestedElementsAndAttributes)
{
    std::string in_xml = "<root id='42'><item enabled='yes'>value</item></root>";
    rapidxml::xml_document<> xml_document;

    xml_document.parse<rapidxml::parse_default>(&in_xml[0]);

    rapidxml::xml_node<>* root_node = xml_document.first_node("root");
    ASSERT_NE(root_node, nullptr);
    ASSERT_NE(root_node->first_attribute("id"), nullptr);
    EXPECT_STREQ(root_node->first_attribute("id")->value(), "42");

    rapidxml::xml_node<>* item_node = root_node->first_node("item");
    ASSERT_NE(item_node, nullptr);
    ASSERT_NE(item_node->first_attribute("enabled"), nullptr);
    EXPECT_STREQ(item_node->first_attribute("enabled")->value(), "yes");
    EXPECT_STREQ(item_node->value(), "value");
}

TEST(RapidXmlParseTest, TranslatesXmlEntities)
{
    std::string in_xml = "<root attr='A&amp;B'>A &lt; B &amp;&amp; C</root>";
    rapidxml::xml_document<> xml_document;

    xml_document.parse<rapidxml::parse_default>(&in_xml[0]);

    rapidxml::xml_node<>* root_node = xml_document.first_node("root");
    ASSERT_NE(root_node, nullptr);
    EXPECT_STREQ(root_node->first_attribute("attr")->value(), "A&B");
    EXPECT_STREQ(root_node->value(), "A < B && C");
}

TEST(RapidXmlParseTest, KeepsSourceUnmodifiedWithNonDestructiveFlag)
{
    std::string in_xml = "<root attr='A&amp;B'>value</root>";
    const std::string original_xml = in_xml;
    rapidxml::xml_document<> xml_document;

    xml_document.parse<rapidxml::parse_non_destructive>(&in_xml[0]);

    EXPECT_EQ(in_xml, original_xml);
    rapidxml::xml_attribute<>* attribute =
        xml_document.first_node("root")->first_attribute("attr");
    ASSERT_NE(attribute, nullptr);
    EXPECT_EQ(std::string(attribute->value(), attribute->value_size()), "A&amp;B");
}

TEST(RapidXmlParseTest, ParsesCommentAndCDataNodesWithFullFlag)
{
    std::string in_xml = "<?xml version='1.0'?><!--note--><root><![CDATA[a < b]]></root>";
    rapidxml::xml_document<> xml_document;

    xml_document.parse<rapidxml::parse_full>(&in_xml[0]);

    rapidxml::xml_node<>* declaration_node = xml_document.first_node();
    ASSERT_NE(declaration_node, nullptr);
    EXPECT_EQ(declaration_node->type(), rapidxml::node_declaration);

    rapidxml::xml_node<>* comment_node = declaration_node->next_sibling();
    ASSERT_NE(comment_node, nullptr);
    EXPECT_EQ(comment_node->type(), rapidxml::node_comment);
    EXPECT_STREQ(comment_node->value(), "note");

    rapidxml::xml_node<>* root_node = xml_document.first_node("root");
    ASSERT_NE(root_node, nullptr);
    rapidxml::xml_node<>* cdata_node = root_node->first_node();
    ASSERT_NE(cdata_node, nullptr);
    EXPECT_EQ(cdata_node->type(), rapidxml::node_cdata);
    EXPECT_STREQ(cdata_node->value(), "a < b");
}

TEST(RapidXmlParseTest, RejectsMismatchedClosingTagsWhenValidationEnabled)
{
    std::string in_xml = "<root><item></root>";
    rapidxml::xml_document<> xml_document;

    EXPECT_THROW(
        xml_document.parse<rapidxml::parse_validate_closing_tags>(&in_xml[0]),
        rapidxml::parse_error);
}

} // namespace
