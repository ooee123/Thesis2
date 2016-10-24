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
    <T> Collection<T> visitEachStatement(Visitor<T, Statement> visitor);
}