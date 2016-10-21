package pdg;

import ast.Statement;

import java.util.Collection;

/**
 * Created by ooee on 10/20/16.
 */
public interface PDGNodeContainsStatementNode {
    Collection<PDGNode<? extends Statement>> getStatementNodes();
}
