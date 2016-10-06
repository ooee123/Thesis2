package ast;

import lombok.NonNull;
import lombok.Value;

/**
 * Created by ooee on 9/29/16.
 */
@Value
public class IterationStatementDeclareFor implements IterationStatement {
    @NonNull private Declaration declaration;
    private Expression condition;
    private Expression iteration;
    @NonNull private Statement statement;

    public String toCode() {
        String conditionString = "", iterationString = "";
        if (condition != null) {
            conditionString = condition.toCode();
        }
        if (iteration != null) {
            iterationString = iteration.toCode();
        }
        return String.format("for (%s %s; %s) {\n%s}", declaration.toCode(), conditionString, iterationString, statement.toCode());
    }
}
