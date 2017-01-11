package ast.statement;

import ast.BlockItem;
import visitor.Visitor;

/**
 * Created by ooee on 9/24/16.
 */
public interface Statement extends BlockItem {
    void visitEachStatement(Visitor<Statement> visitor);
}