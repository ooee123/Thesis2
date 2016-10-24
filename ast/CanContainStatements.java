package ast;

import pdg.PDGNode;
import visitor.Visitor;

import java.util.Collection;

/**
 * Created by ooee on 9/24/16.
 */
public interface CanContainStatements {
    Collection<Statement> getStatementNodes();


}
