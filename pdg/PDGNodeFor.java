package pdg;

import ast.expression.Expression;
import ast.statement.impl.IterationStatementFor;
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
public class PDGNodeFor extends PDGNodeContainsStatementNode<IterationStatementFor> {
    private PDGNode<? extends Statement> body;

    public PDGNodeFor(IterationStatementFor statement, PDGNode<? extends Statement> body) {
        this(statement, body, false);
    }

    public PDGNodeFor(IterationStatementFor statement, PDGNode<? extends Statement> body, boolean required) {
        super(statement, required);
        this.body = body;
    }

    @Override
    public IterationStatementFor sort(PDGSorter sorter) {
        Expression condition = blockItem.getCondition();
        Statement statement = body.sort(sorter);
        return new IterationStatementFor(blockItem.getInitial(), blockItem.getCondition(), blockItem.getIteration(), statement, blockItem.getOriginalLine());
    }

    public String toCode(PDGSorterDefault sorter) {
        return "";
    }

    @Override
    public Collection<PDGNode<? extends Statement>> getStatementNodes() {
        return Lists.newArrayList(body);
    }
}
