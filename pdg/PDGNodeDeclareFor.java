package pdg;

import ast.statement.impl.IterationStatementDeclareFor;
import ast.statement.Statement;
import com.google.common.collect.Lists;
import lombok.EqualsAndHashCode;
import lombok.Value;

import java.util.Collection;

/**
 * Created by ooee on 10/9/16.
 */
@Value
@EqualsAndHashCode(callSuper = true)
public class PDGNodeDeclareFor extends PDGNodeContainsStatementNode<IterationStatementDeclareFor> {
    private PDGNode<? extends Statement> body;

    public PDGNodeDeclareFor(IterationStatementDeclareFor statement, PDGNode<? extends Statement> body) {
        this(statement, body, false);
    }

    public PDGNodeDeclareFor(IterationStatementDeclareFor statement, PDGNode<? extends Statement> body, boolean required) {
        super(statement, required);
        this.body = body;
    }

    @Override
    public IterationStatementDeclareFor sort(PDGSorter sorter) {
        Statement statement = body.sort(sorter);
        return new IterationStatementDeclareFor(blockItem.getVariableDeclaration(), blockItem.getCondition(), blockItem.getIteration(), statement);
    }

    public String toCode(PDGSorterDefault sorter) {
        return "";
    }

    @Override
    public Collection<PDGNode<? extends Statement>> getStatementNodes() {
        return Lists.newArrayList(body);
    }
}
