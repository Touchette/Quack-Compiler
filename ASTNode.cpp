#include <map>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include "ASTNode.h"

namespace AST {

    void ASTNode::insert(ASTNode *node) {
        /* Docstring TBD */

        printf("-------- INSERT \n");
        /* If this type of node is not in the map yet, need a new vector for it to be inserted */
        if (this->children.count(node->type) == 0) {
            printf("Insert: we don't have this type yet, create new vec and shove it in\n");
            std::vector<ASTNode *> insertedNodeVector;
            insertedNodeVector.push_back(node);

            this->children.insert(std::map<Type, std::vector<ASTNode *> >::value_type(node->type, insertedNodeVector));
        }
        /* Else, find that type in the map and push this new node into its vector */
        else {
            printf("Insert; we already have that type, shove it in the vec\n");
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
            printf("Insert: not in order, shove it in the order vec\n");
            order.push_back(node->type);
        }
        std::cout << std::endl;
    }

    ASTNode * ASTNode::get(Type type) {
        /* Docstring TBD */

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
        /* Docstring TBD */

        /* Finds a given type of node in the map, returns its associated vector */
        std::map<Type, std::vector<ASTNode *> >::iterator it = children.find(type);
        if (it == children.end()) {
            fprintf(stderr, "fucked up \n");
        }
        else {
            fprintf(stderr, "found thing, returning \n");
            return it->second;
        }
    }

    // /* -~-~-~- JSON printing methods -~-~-~- */
    // void ASTNode::json_child (std::ostream &out, AST_print_context &ctx, std::string field, ASTNode &child, char sep) {
    //     json_indent(out, ctx);
    //     out << "\"" << field << "\" : ";
    //     child.json(out, ctx);
    //     out << sep;
    // }

    // /* The head element looks like { "kind" : "block", */
    // void ASTNode::json_head  (std::ostream &out, AST_print_context &ctx, std::string node_kind) {
    //     json_indent(out, ctx);
    //     out << "{ \"kind\" : \"" << node_kind << "\"," ;
    //     ctx.indent();  // one level more for children
    //     return;
    // }

    // void ASTNode::json_close (std::ostream &out, AST_print_context &ctx); {
    //     // json_indent(out, ctx);
    //     out << "}";
    //     ctx.dedent();
    // }

    // void ASTNode::json_indent(std::ostream &out, AST_print_context &ctx); {
    //     if (ctx.indent_ > 0) {
    //         out << std::endl;
    //     }

    //     for (int i=0; i<ctx.indent_; ++i) {
    //         out << "    ";
    //     }
    // }

}