package ast;

import lombok.NonNull;
import lombok.Value;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class ConditionalExpressionImpl implements ConditionalExpression {
    @NonNull private LogicalOrExpression logicalOrExpression;
    private Expression trueExpression;
    private ConditionalExpression falseExpression;
}
