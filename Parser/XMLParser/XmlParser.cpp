/*
 Copyright (c) 2012, Esteban Pellegrino
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the <organization> nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "XmlParser.hpp"
#include "tinyxml.h"

using namespace std;

//int dump_attribs_to_stdout(TiXmlElement* pElement, unsigned int indent)
//{
//	if ( !pElement ) return 0;
//
//	TiXmlAttribute* pAttrib=pElement->FirstAttribute();
//	int i=0;
//	int ival;
//	double dval;
//	const char* pIndent=getIndent(indent);
//	printf("\n");
//	while (pAttrib)
//	{
//		printf( "%s%s: value=[%s]", pIndent, pAttrib->Name(), pAttrib->Value());
//
//		if (pAttrib->QueryIntValue(&ival)==TIXML_SUCCESS)    printf( " int=%d", ival);
//		if (pAttrib->QueryDoubleValue(&dval)==TIXML_SUCCESS) printf( " d=%1.1f", dval);
//		printf( "\n" );
//		i++;
//		pAttrib=pAttrib->Next();
//	}
//	return i;
//}

//void dump_to_stdout( TiXmlNode* pParent, unsigned int indent = 0 )
//{
//	if ( !pParent ) return;
//
//	TiXmlNode* pChild;
//	TiXmlText* pText;
//	int t = pParent->Type();
//	printf( "%s", getIndent(indent));
//	int num;
//
//	switch ( t )
//	{
//	case TiXmlNode::DOCUMENT:
//		printf( "Document" );
//		break;
//
//	case TiXmlNode::ELEMENT:
//		printf( "Element [%s]", pParent->Value() );
//		num=dump_attribs_to_stdout(pParent->ToElement(), indent+1);
//		switch(num)
//		{
//			case 0:  printf( " (No attributes)"); break;
//			case 1:  printf( "%s1 attribute", getIndentAlt(indent)); break;
//			default: printf( "%s%d attributes", getIndentAlt(indent), num); break;
//		}
//		break;
//
//	case TiXmlNode::COMMENT:
//		printf( "Comment: [%s]", pParent->Value());
//		break;
//
//	case TiXmlNode::UNKNOWN:
//		printf( "Unknown" );
//		break;
//
//	case TiXmlNode::TEXT:
//		pText = pParent->ToText();
//		printf( "Text: [%s]", pText->Value() );
//		break;
//
//	case TiXmlNode::DECLARATION:
//		printf( "Declaration" );
//		break;
//	default:
//		break;
//	}
//	printf( "\n" );
//	for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
//	{
//		dump_to_stdout( pChild, indent+1 );
//	}
//}

namespace Helios {

void getGeometryInformation(const string& geo_file,
		                    vector<Geometry::SurfaceDefinition>& sur_def,
		                    vector<Geometry::CellDefinition>& cell_def) const {
	/* Open document */
	TiXmlDocument doc(geo_file.c_str());
	bool loadOkay = doc.LoadFile();
	if (loadOkay) {

	} else {
		cerr << "[@] Failed to load file : " << geo_file;
	}
}

} /* namespace Helios */
