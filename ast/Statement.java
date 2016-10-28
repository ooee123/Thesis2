package ast;

import pdg.PDGNode;
import visitor.Visitor;

import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

/**
 * Created by ooee on 9/24/16.
 */
public interface Statement extends BlockItem {
    void visitEachStatement(Visitor<Statement> visitor);
}