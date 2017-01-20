package ast.expression;

import ast.BaseElement;
import ast.expression.impl.PostfixExpressionInvocationImpl;
import com.google.common.collect.Sets;
import visitor.Visitor;

import java.util.*;

/**
 * Created by ooee on 9/25/16.
 */
public interface Expression extends BaseElement {

    @Override
    default String toCode(boolean showOriginalLine) {
        return toCode();
    }

    String toCode();

    default boolean hasExit() {
        for (PostfixExpressionInvocationImpl postfixExpressionInvocation : getInvocations()) {
            String functionName = postfixExpressionInvocation.getFunction().getIdentifier();
            if (functionName.equals("exit")) {
                return true;
            }
        }
        return false;
    }

    Set<String> getVariables();

    Set<String> getDependentVariables();

    Set<String> getGuaranteedChangedVariables();

    default Set<String> getPotentiallyChangedVariables() {
        return Sets.newHashSet();
    }

    Set<PostfixExpressionInvocationImpl> getInvocations();

    void visitNestedExpressions(Visitor<Expression> visitor);

    default <T> Set<T> multiGetSet(java.util.function.Function<Expression, Set<T>> f, Expression... expressions) {
        return multiGetCollection(new HashSet<>(), f, expressions);
    }

    default <T> Set<T> multiGetIdentitySet(java.util.function.Function<Expression, Set<T>> f, Expression... expressions) {
        return multiGetCollection(Sets.newIdentityHashSet(), f, expressions);
    }

    default <T, C extends Collection<T>> C multiGetCollection(C collection, java.util.function.Function<Expression, C> f, Expression... expressions) {
        for (Expression expression : expressions) {
            collection.addAll(f.apply(expression));
        }
        return collection;
    }

    default Set<String> multiGetVariables(Expression... expressions) {
        return multiGetSet(exp -> exp.getVariables(), expressions);
    }

    default Set<String> multiGetDependentVariables(Expression... expressions) {
        return multiGetSet(exp -> exp.getDependentVariables(), expressions);
    }

    default Set<String> multiGetGuaranteedChangedVariables(Expression... expressions) {
        return multiGetSet(exp -> exp.getGuaranteedChangedVariables(), expressions);
    }

    default Set<String> multiGetPotentiallyChangedVariables(Expression... expressions) {
        return multiGetSet(exp -> exp.getPotentiallyChangedVariables(), expressions);
    }

    default Set<PostfixExpressionInvocationImpl> multiGetInvocations(Expression... expressions) {
        return multiGetIdentitySet(exp -> exp.getInvocations(), expressions);
    }
}
