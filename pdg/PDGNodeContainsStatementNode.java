package pdg;

import ast.statement.Statement;

import java.util.Collection;

/**
 * Created by ooee on 10/20/16.
 */
public abstract class PDGNodeContainsStatementNode<T extends Statement> extends PDGNode<T> {

    public PDGNodeContainsStatementNode(T blockItem) {
        this(blockItem, false);
    }

    public PDGNodeContainsStatementNode(T blockItem, boolean required) {
        super(blockItem, required);
    }

    public abstract Collection<PDGNode<? extends Statement>> getStatementNodes();
}
