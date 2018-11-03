#include <map>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include "ASTNode.h"

std::set<Type> HeadTypes = {
        PROGRAM, FORMAL_ARG, METHOD, ASSIGN, RETURN, IF, 
        TYPECASE, TYPE_ALTERNATIVE, LOAD, IDENT, CLASS, 
        CALL, CONSTRUCTOR, INTCONST, STRCONST, BINOP
};

std::set<Type> SeqTypes = {
        BLOCK, CLASSES, FORMAL_ARGS, METHODS, 
        TYPE_ALTERNATIVES, ACTUAL_ARGS
};

bool isSeqType(Type type) {
    const bool is_in = SeqTypes.find(type) != SeqTypes.end();
    return is_in;
}

std::string typeString(Type type) {
    return TypeString[type];
}

namespace AST {

    /* ======================== */
    /* Inserting & Getting Data */
    /* ======================== */

    void ASTNode::insert(ASTNode *node) {
        /* If this type of node is not in the map yet, need a new vector for it to be inserted */
        if (this->children.count(node->type) == 0) {
            std::vector<ASTNode *> insertedNodeVector;
            insertedNodeVector.push_back(node);

            this->children.insert(std::map<Type, std::vector<ASTNode *> >::value_type(node->type, insertedNodeVector));
        }
        /* Else, find that type in the map and push this new node into its vector */
        else {
            std::map<Type, std::vector<ASTNode *> >::iterator it = this->children.find(node->type);
            if (it != this->children.end()) {
                it->second.push_back(node);
            }
        }

        /* If this type of node is being inserted for the first time,  it needs to go
         * into the "order" vector to know when it was inserted, this is for the json
         * printing
         */
        std::vector<Type>::iterator it = std::find(order.begin(), order.end(), node->type);
        if (it == order.end()) {
            order.push_back(node->type);
        }
    }

    ASTNode * ASTNode::get(Type type) {
        /* Finds a given type of node in the map, returns the first node in its vector.
         * This is used when getting types of nodes that only have one identifier, such
         * as a class name, etc.
         */
        std::map<Type, std::vector<ASTNode *> >::iterator it = this->children.find(type);
        if (it != this->children.end()) {
            return it->second.front();
        }
    }

    std::vector<ASTNode *> ASTNode::getSeq(Type type) {
        /* Finds a given type of node in the map, returns its associated vector */
        std::map<Type, std::vector<ASTNode *> >::iterator it = children.find(type);
        if (it == children.end()) { }
        else {
            return it->second;
        }
    }

    /* ================================ */
    /* -~-~-~- Main JSON method -~-~-~- */
    /* ================================ */

    void ASTNode::json(std::ostream &out, AST_print_context &ctx) {
        if (isSeqType(this->type)) {
            jsonSeq(out, ctx);
        } else {
            json_head(typeString(this->type), out, ctx);
            if (!this->name.empty()) {
                out << "\"text_\" : \"" << this->name << "\"";
            } else if (this->value) {
                out << "\"value_\" : \"" << this->value << "\"";
            }

            auto sep = "";
            for (Type t : this->order) {
                std::vector<ASTNode*> subchildren = this->getSeq(t);
                for (ASTNode* node : subchildren) {
                    node->json(out, ctx);
                    std::cout << "Here\n";
                    if (!node->name.empty()) {
                        std::cout << "Reached\n";
                        out << "\"text_\" : \"" << node->name << "\"";
                    } else if (node->value) {
                        out << "\"value_\" : \"" << node->value << "\"";
                    }
                }
            }
            json_close(out, ctx);
        }
    }


    /* ===================================== */
    /* -~-~-~- JSON printing methods -~-~-~- */
    /* ===================================== */

    void ASTNode::json_child (std::ostream &out, AST_print_context &ctx, std::string field, ASTNode &child, char sep=',') {
        json_indent(out, ctx);
        out << "\"" << field << "\" : ";
        child.json(out, ctx);
        out << sep;
    }

    /* The head element looks like { "kind" : "block", */
    void ASTNode::json_head(std::string node_kind, std::ostream &out, AST_print_context &ctx) {
        json_indent(out, ctx);
        out << "{\"kind\" : \"" << node_kind << "\", " ;
        ctx.indent();
        return;
    }

    void ASTNode::json_close(std::ostream &out, AST_print_context &ctx) {
        out << "}";
        ctx.dedent();
    }

    /* Indent to a given level */
    void ASTNode::json_indent(std::ostream &out, AST_print_context &ctx) {
        if (ctx.indent_ > 0) {
            out << std::endl;
        }
        for (int i=0; i < ctx.indent_; ++i) {
            out << "    ";
        }
    }

    void ASTNode::jsonSeq(std::ostream &out, AST_print_context &ctx) {
        json_head(typeString(this->type), out, ctx);

        out << "\"elements_\" : [";
        auto sep = "";
        for (Type t : this->order) {
            std::vector<ASTNode*> subchildren = this->getSeq(t);
            for (ASTNode* node : subchildren) {
                node->json(out, ctx);
            }
        }
        out << "]";
        json_close(out, ctx);
    }

}