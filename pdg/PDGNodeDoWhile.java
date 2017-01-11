package pdg;

import ast.expression.Expression;
import ast.statement.impl.IterationStatementDoWhile;
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
public class PDGNodeDoWhile extends PDGNodeContainsStatementNode<IterationStatementDoWhile> {
    private PDGNode<? extends Statement> body;

    public PDGNodeDoWhile(IterationStatementDoWhile statement, PDGNode<? extends Statement> body) {
        this(statement, body, false);
    }

    public PDGNodeDoWhile(IterationStatementDoWhile statement, PDGNode<? extends Statement> body, boolean required) {
        super(statement, required);
        this.body = body;
    }

    public IterationStatementDoWhile sort(PDGSorter sorter) {
        Expression condition = blockItem.getCondition();
        Statement statement = body.sort(sorter);
        return new IterationStatementDoWhile(statement, condition);
    }

    public String toCode(PDGSorterDefault sorter) {
        return "";
    }

    @Override
    public Collection<PDGNode<? extends Statement>> getStatementNodes() {
        return Lists.newArrayList(body);
    }
}
