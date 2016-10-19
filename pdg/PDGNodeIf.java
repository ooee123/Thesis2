package pdg;

import ast.CompoundStatement;
import ast.SelectionStatementIf;
import ast.Statement;
import lombok.Value;

import java.util.Collection;

/**
 * Created by ooee on 10/9/16.
 */
@Value
public class PDGNodeIf extends PDGNode<SelectionStatementIf> {
    private PDGNode thenNode;
    private PDGNode elseNode;

    public PDGNodeIf(SelectionStatementIf selectionStatementIf, PDGNode thenNode, PDGNode elseNode) {
        this(selectionStatementIf, thenNode, elseNode, false);
    }

    public PDGNodeIf(SelectionStatementIf selectionStatementIf, PDGNode thenNode, PDGNode elseNode, boolean required) {
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
            elseStatement = (Statement)elseNode.sort(sorter);
        }
        return new SelectionStatementIf(blockItem.getCondition(), (Statement)thenNode.sort(sorter), elseStatement);
    }
}
