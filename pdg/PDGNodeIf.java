package pdg;

import ast.SelectionStatementIf;
import ast.Statement;
import lombok.EqualsAndHashCode;
import lombok.NonNull;
import lombok.Value;

import java.util.ArrayList;
import java.util.Collection;

/**
 * Created by ooee on 10/9/16.
 */
@Value
@EqualsAndHashCode(callSuper = true)
public class PDGNodeIf extends PDGNodeContainsStatementNode<SelectionStatementIf> {
    @NonNull private PDGNode<? extends Statement> thenNode;
    private PDGNode<? extends Statement> elseNode;

    public PDGNodeIf(SelectionStatementIf selectionStatementIf, PDGNode<? extends Statement> thenNode, PDGNode<? extends Statement> elseNode) {
        this(selectionStatementIf, thenNode, elseNode, false);
    }

    public PDGNodeIf(SelectionStatementIf selectionStatementIf, PDGNode<? extends Statement> thenNode, PDGNode<? extends Statement> elseNode, boolean required) {
        super(selectionStatementIf, required);
        this.thenNode = thenNode;
        this.elseNode = elseNode;
    }

    public String toCode(PDGSorter sorter) {
        return "";
    }

    @Override
    public SelectionStatementIf sort(PDGSorter sorter) {
        Statement elseStatement = null;
        if (elseNode != null) {
            elseStatement = elseNode.sort(sorter);
        }
        return new SelectionStatementIf(blockItem.getCondition(), thenNode.sort(sorter), elseStatement);
    }

    @Override
    public Collection<PDGNode<? extends Statement>> getStatementNodes() {
        Collection<PDGNode<? extends Statement>> statementNodes = new ArrayList<>();
        statementNodes.add(thenNode);
        if (elseNode != null) {
            statementNodes.add(elseNode);
        }
        return statementNodes;
    }
/*
    public void removeEmptyCompoundStatement() {
        if (elseNode instanceof PDGNodeCompoundStatement) {
            if (((PDGNodeCompoundStatement) elseNode).getBody().isEmpty()) {
                elseNode = null;
            }
        }
    }
    */
}
