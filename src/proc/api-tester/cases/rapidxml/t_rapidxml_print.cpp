#include <string>

#include "gtest/gtest.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_ext.hpp"

namespace {

TEST(RapidXmlPrintTest, PrintsCompactXmlAndEscapesEntities)
{
    rapidxml::xml_document<> xml_document;
    rapidxml::xml_node<>* root_node = xml_document.allocate_node(rapidxml::node_element, "root");
    rapidxml::xml_attribute<>* attribute = xml_document.allocate_attribute("value", "A&B");
    rapidxml::xml_node<>* text_node =
        xml_document.allocate_node(rapidxml::node_data, nullptr, "A < B");

    xml_document.append_node(root_node);
    root_node->append_attribute(attribute);
    root_node->append_node(text_node);

    std::string out_xml;
    rapidxml::print(std::back_inserter(out_xml), xml_document, rapidxml::print_no_indenting);

    EXPECT_EQ(out_xml, "<root value=\"A&amp;B\">A &lt; B</root>");
}

TEST(RapidXmlPrintTest, PrintsIndentedXml)
{
    rapidxml::xml_document<> xml_document;
    rapidxml::xml_node<>* root_node = xml_document.allocate_node(rapidxml::node_element, "root");
    rapidxml::xml_node<>* child_node = xml_document.allocate_node(rapidxml::node_element, "child");

    xml_document.append_node(root_node);
    root_node->append_node(child_node);

    std::string out_xml;
    rapidxml::print(std::back_inserter(out_xml), xml_document);

    EXPECT_NE(out_xml.find("<root>"), std::string::npos);
    EXPECT_NE(out_xml.find("\t<child/>"), std::string::npos);
}

} // namespace
