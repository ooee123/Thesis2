package ast;

import lombok.Value;

import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class UnaryExpressionImpl implements UnaryExpression {
    @Override
    public Set<String> getLValues() {
        return unaryExpression.getLValues();
    }

    public enum IncrementOperator {
        PREFIX_INCREMENT,
        PREFIX_DECREMENT;
    }

    private IncrementOperator incrementOperator;
    private UnaryExpression unaryExpression;
}
