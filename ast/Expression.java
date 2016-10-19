package ast;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

/**
 * Created by ooee on 9/25/16.
 */
public interface Expression extends BaseElement {
    Set<String> getVariables();

    Set<String> getDependentVariables();

    Set<String> getGuaranteedChangedVariables();

    default Set<String> getPotentiallyChangedVariables() {
        return Collections.emptySet();
    }

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

    default Set<String> multiGetGuaranteedChangedVariables(Expression... expressions) {
        return multiGet(exp -> exp.getGuaranteedChangedVariables(), expressions);
    }

    default Set<String> multiGetPotentiallyChangedVariables(Expression... expressions) {
        return multiGet(exp -> exp.getPotentiallyChangedVariables(), expressions);
    }

    default Set<PostfixExpressionInvocationImpl> multiGetInvocations(Expression... expressions) {
        return multiGet(exp -> exp.getInvocations(), expressions);
    }


}
