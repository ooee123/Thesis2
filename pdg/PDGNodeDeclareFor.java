package pdg;

import ast.Expression;
import ast.IterationStatementDeclareFor;
import ast.IterationStatementFor;
import ast.Statement;
import lombok.Value;

/**
 * Created by ooee on 10/9/16.
 */
@Value
public class PDGNodeDeclareFor extends PDGNode<IterationStatementDeclareFor> {
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
        return new IterationStatementDeclareFor(blockItem.getDeclaration(), blockItem.getCondition(), blockItem.getIteration(), statement);
    }

    public String toCode(PDGSorterDefault sorter) {
        return "";
    }
}
