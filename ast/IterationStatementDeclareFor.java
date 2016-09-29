package ast;

import lombok.Value;

/**
 * Created by ooee on 9/29/16.
 */
@Value
public class IterationStatementDeclareFor implements IterationStatement {
    private Declaration declaration;
    private Expression condition;
    private Expression iteration;
}
