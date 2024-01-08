/*
 * CMLReader for InChI
 * Developed in collaboration with NIST and IUPAC
 * Version 1.0
 * Software version 1.0
 * 2004
 * Developed by Peter Murray-Rust and Simon (Billy) Tyrrell
 * Unilever Centre for Molecular Sciences Informatics
 * Department of Chemistry, University of Cambridge
 * Cambridge, CB2 1EW, UK
 *
 * CMLReader is free software 
 * you can redistribute this software and/or modify it under the terms of
 * the GNU Lesser General Public License as published by the Free Software
 * Foundation:
 * http://www.opensource.org/licenses/lgpl-license.php
 */


#include "pragma.hpp"
#include "CMLMetadataList.hpp"
#include "CMLSaxHandler.hpp"

using namespace std;
namespace CML {

    CMLMetadataList::CMLMetadataList() :
        CMLBase() {
        init();
    }

    CMLMetadataList::CMLMetadataList(CMLDocument* documentPtr) :
        CMLBase(documentPtr, C_METADATALIST) {
        init();
    }

    CMLMetadataList::CMLMetadataList(const CMLMetadataList& m) :
        CMLBase((CMLBase) m) {
    }

    void CMLMetadataList::init() {
//        metadataPtrVector.clear();
    }

    CMLMetadataList::~CMLMetadataList() {
    }

    // not yet finished
    void CMLMetadataList::writeXML(ostream &os) {
        const Serializer* const serializerPtr = getSerializerPtr();
        serializerPtr->writeStartTagStart(os, C_METADATALIST);
        serializerPtr->writeAttribute(os, C_TITLE, "generated automatically from Openbabel");
        serializerPtr->writeStartTagEnd(os);
        os << endl;

        serializerPtr->writeStartTagStart(os, C_METADATA);
        serializerPtr->writeAttribute(os, C_NAME, DC_CREATOR);
        serializerPtr->writeAttribute(os, C_CONTENT, "OpenBabel version 1-100.1");
        serializerPtr->writeCombinedTagEnd(os);

        serializerPtr->writeStartTagStart(os, C_METADATA);
        serializerPtr->writeAttribute(os, C_NAME, DC_DESCRIPTION);
        serializerPtr->writeAttribute(os, C_CONTENT, "Conversion of legacy filetype to CML");
        serializerPtr->writeCombinedTagEnd(os);

        serializerPtr->writeStartTagStart(os, C_METADATA);
        serializerPtr->writeAttribute(os, C_NAME, DC_IDENTIFIER);
        serializerPtr->writeAttribute(os, C_CONTENT, "Unknown");
        serializerPtr->writeCombinedTagEnd(os);

        serializerPtr->writeStartTagStart(os, C_METADATA);
        serializerPtr->writeAttribute(os, C_NAME, DC_RIGHTS);
        serializerPtr->writeAttribute(os, C_CONTENT, "unknown");
        serializerPtr->writeCombinedTagEnd(os);

        serializerPtr->writeStartTagStart(os, C_METADATA);
        serializerPtr->writeAttribute(os, C_NAME, DC_TYPE);
        serializerPtr->writeAttribute(os, C_CONTENT, "chemistry");
        serializerPtr->writeCombinedTagEnd(os);

        serializerPtr->writeStartTagStart(os, C_METADATA);
        serializerPtr->writeAttribute(os, C_NAME, DC_CONTRIBUTOR);
        serializerPtr->writeAttribute(os, C_CONTENT, "unknown");
        serializerPtr->writeCombinedTagEnd(os);

        serializerPtr->writeStartTagStart(os, C_METADATA);
        serializerPtr->writeAttribute(os, C_NAME, DC_CREATOR);
        serializerPtr->writeAttribute(os, C_CONTENT, "Openbabel V1-100.1");
        serializerPtr->writeCombinedTagEnd(os);

        serializerPtr->writeStartTagStart(os, C_METADATA);
        serializerPtr->writeAttribute(os, C_NAME, DC_DATE);
        string time;
//        getTimestr(time);
        serializerPtr->writeAttribute(os, C_CONTENT, time);
        serializerPtr->writeCombinedTagEnd(os);

        serializerPtr->writeStartTagStart(os, C_METADATA);
        serializerPtr->writeAttribute(os, C_NAME, CMLM_STRUCTURE);
        serializerPtr->writeAttribute(os, C_CONTENT, "yes");
        serializerPtr->writeCombinedTagEnd(os);

        serializerPtr->writeEndTag(os, C_METADATALIST);
    }
}

