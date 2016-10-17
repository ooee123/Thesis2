package ast;

import java.lang.reflect.Method;
import java.util.Collection;
import java.util.HashSet;
import java.util.Set;
import java.util.function.*;

/**
 * Created by ooee on 9/25/16.
 */
public interface Expression extends BaseElement {
    Set<String> getVariables();

    Set<String> getDependentVariables();

    Set<String> getChangedVariables();

    Set<PostfixExpressionInvocationImpl> getInvocations();

    default <T> Set<T> multiGet(java.util.function.Function<Expression, Set<T>> f, Expression... expressions) {
        Set<T> set = new HashSet<>();
        for (Expression expression : expressions) {
            set.addAll(f.apply(expression));
        }
        return set;
    }

    default Set<String> multiGetVariables(Expression... expressions) {
        return multiGet(exp -> exp.getVariables(), expressions);
    }

    default Set<String> multiGetDependentVariables(Expression... expressions) {
        return multiGet(exp -> exp.getDependentVariables(), expressions);
    }

    default Set<String> multiGetChangedVariables(Expression... expressions) {
        return multiGet(exp -> exp.getChangedVariables(), expressions);
    }

    default Set<PostfixExpressionInvocationImpl> multiGetInvocations(Expression... expressions) {
        return multiGet(exp -> exp.getInvocations(), expressions);
    }
}
