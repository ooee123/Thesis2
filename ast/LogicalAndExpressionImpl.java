package ast;

import lombok.Value;
import visitor.Visitor;

import java.util.*;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class LogicalAndExpressionImpl implements LogicalAndExpression {
    public static final String AND = "&&";

    private Collection<InclusiveBitwiseOrExpression> inclusiveBitwiseOrExpressions;

    @Override
    public String toCode() {
        List<String> codes = collectionToCode(inclusiveBitwiseOrExpressions);
        return String.join(AND, codes);
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return multiGetGuaranteedChangedVariables(inclusiveBitwiseOrExpressions.toArray(new Expression[0]));
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        for (InclusiveBitwiseOrExpression inclusiveBitwiseOrExpression : inclusiveBitwiseOrExpressions) {
            variables.addAll(inclusiveBitwiseOrExpression.getVariables());
        }
        return variables;
    }

    @Override
    public Set<String> getDependentVariables() {
        return multiGetDependentVariables(inclusiveBitwiseOrExpressions.toArray(new Expression[0]));
    }

    @Override
    public List<PostfixExpressionInvocationImpl> getInvocations() {
        return multiGetInvocations(inclusiveBitwiseOrExpressions.toArray(new Expression[0]));
    }

    @Override
    public void visitNestedExpressions(Visitor<Expression> visitor) {
        for (InclusiveBitwiseOrExpression inclusiveBitwiseOrExpression : inclusiveBitwiseOrExpressions) {
            visitor.visit(inclusiveBitwiseOrExpression);
        }
    }
}
