package ast;

import lombok.NonNull;
import lombok.Value;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class ConditionalExpression implements AssignmentExpression {
    @NonNull private LogicalOrExpression logicalOrExpression;
    private Expression trueExpression;
    private ConditionalExpression falseExpression;
}
