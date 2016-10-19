package visitor;

import ast.*;
import ast.Function;
import ast.type.PointerType;
import com.google.common.collect.Sets;
import lombok.AllArgsConstructor;
import lombok.Value;
import pdg.PDGNode;

import java.util.*;
import java.util.stream.Collectors;

@Value
public class PDGGenerationVisitor {

    public Collection<PDGNode> getPDGNodes() {
        return allNodes.values();
    }

    private Map<BlockItem, PDGNode> allNodes = new IdentityHashMap<>();

    public void link(BlockItem from, BlockItem to) {
        if (!allNodes.containsKey(from)) {
            allNodes.put(from, new PDGNode(from, from.isCritical()));
        }
        if (!allNodes.containsKey(to)) {
            allNodes.put(to, new PDGNode(to, to.isCritical()));
        }
        PDGNode fromNode = allNodes.get(from), toNode = allNodes.get(to);
        fromNode.getIsADependencyFor().add(toNode);
        toNode.getDependsOn().add(fromNode);
    }

    public void printAdjacencyMatrix() {
        int i = 0;
        String label;
        System.out.println("Adjacency Matrix");
        Map<PDGNode, String> labels = new IdentityHashMap<>();
        for (PDGNode pdgNode : allNodes.values()) {
            label = String.valueOf(Character.valueOf((char) ('A' + i)));
            labels.put(pdgNode, label);
            System.out.println(label + " := " + pdgNode.getBlockItem().toCode());
            i++;
        }

        for (PDGNode pdgNode : allNodes.values()) {
            String source = labels.get(pdgNode);
            System.out.print(source + ": ");
            List<String> destLabels = pdgNode.getIsADependencyFor().stream().map(node -> labels.get(node)).collect(Collectors.toList());
            System.out.println(String.join(", ", destLabels));
        }
        System.out.println("End Matrix");
    }


    @Value
    @AllArgsConstructor
    private class Dependencies {
        private Set<String> dependentVariables;
        private Set<String> guaranteedChangedVariables;
        private Set<String> potentiallyChangedVariables;

        Dependencies() {
            dependentVariables = Collections.emptySet();
            guaranteedChangedVariables = Collections.emptySet();
            potentiallyChangedVariables = Collections.emptySet();
        }

        Dependencies add(Dependencies d) {
            Set<String> otherDependentVariables = new HashSet<>(d.dependentVariables);
            otherDependentVariables.removeAll(this.guaranteedChangedVariables);
            this.dependentVariables.addAll(otherDependentVariables);
            this.potentiallyChangedVariables.removeAll(d.guaranteedChangedVariables);
            this.guaranteedChangedVariables.removeAll(d.potentiallyChangedVariables);
            this.guaranteedChangedVariables.addAll(d.guaranteedChangedVariables);
            this.potentiallyChangedVariables.addAll(d.potentiallyChangedVariables);
            return this;
        }
    }

    private Dependencies nullableExpression(Expression expression, Dependencies dependencies) {
        if (expression != null) {
            Set<String> rightVariables = expression.getDependentVariables();
            rightVariables.removeAll(dependencies.guaranteedChangedVariables);
            dependencies.dependentVariables.addAll(rightVariables);
            dependencies.guaranteedChangedVariables.removeAll(expression.getPotentiallyChangedVariables());
            dependencies.potentiallyChangedVariables.removeAll(expression.getGuaranteedChangedVariables());
            dependencies.guaranteedChangedVariables.addAll(expression.getGuaranteedChangedVariables());
            dependencies.potentiallyChangedVariables.addAll(expression.getPotentiallyChangedVariables());
        }
        return dependencies;
    }

    public void visit(Program p) {
        for (Function function : p.getFunction()) {
            visit(function);
        }
        printAdjacencyMatrix();
    }

    public Dependencies visit(Function function) {
        Dependencies functionDependencies = visit(function.getCompoundStatement());
        for (Parameter parameter : function.getParameterList().getParameters()) {
            // If the type is a pointer, mark it as changed.
            //functionDependencies.


            if (parameter.getType() instanceof PointerType && functionDependencies.guaranteedChangedVariables.contains(parameter.getFormalParameterName())) {
                // Pointer value changed

            }
        }
        visit(function.getCompoundStatement());
        //functionDependencies.getGuaranteedChangedVariables()
        return null;
    }

    public Dependencies visit(Statement statement) {
        if (statement instanceof CompoundStatement) {
            return visit(((CompoundStatement) statement));
        } else if (statement instanceof ExpressionStatement) {
            return visit(((ExpressionStatement) statement));
        } else if (statement instanceof SelectionStatementIf) {
            return visit(((SelectionStatementIf) statement));
        } else if (statement instanceof IterationStatementFor) {
            return visit(((IterationStatementFor) statement));
        } else if (statement instanceof IterationStatementDeclareFor) {
            return visit(((IterationStatementDeclareFor) statement));
        } else if (statement instanceof IterationStatementWhile) {
            return visit(((IterationStatementWhile) statement));
        } else if (statement instanceof IterationStatementDoWhile) {
            return visit(((IterationStatementDoWhile) statement));
        } else {
            return new Dependencies(statement.getDependantVariables(), statement.getGuaranteedChangedVariables(), statement.getPotentiallyChangedVariables());
        }
    }

    private Dependencies visit(IterationStatementFor statement) {
        Dependencies dependencies = new Dependencies();
        nullableExpression(statement.getInitial(), dependencies);
        nullableExpression(statement.getCondition(), dependencies);
        Dependencies bodyDependencies = visit(statement.getStatement());

        bodyDependencies.dependentVariables.removeAll(dependencies.getPotentiallyChangedVariables());
        dependencies.potentiallyChangedVariables.addAll(bodyDependencies.getGuaranteedChangedVariables());
        dependencies.potentiallyChangedVariables.addAll(bodyDependencies.getPotentiallyChangedVariables());
        dependencies.dependentVariables.addAll(bodyDependencies.getDependentVariables());

        Dependencies iterationDependencies = nullableExpression(statement.getIteration(), new Dependencies());
        dependencies.dependentVariables.addAll(iterationDependencies.getDependentVariables());
        dependencies.potentiallyChangedVariables.addAll(iterationDependencies.getGuaranteedChangedVariables());
        dependencies.potentiallyChangedVariables.addAll(iterationDependencies.getPotentiallyChangedVariables());
        return dependencies;
    }

    private Dependencies visit(IterationStatementDeclareFor statement) {
        Set<String> declaredVariables = new HashSet<>();
        Dependencies dependencies = new Dependencies();
        for (Declaration.DeclaredVariable declaredVariable : statement.getDeclaration().getDeclaredVariables()) {
            if (declaredVariable.getInitializer() != null) {
                nullableExpression(declaredVariable.getInitializer(), dependencies);
                declaredVariables.add(declaredVariable.getIdentifier());
            }
        }
        nullableExpression(statement.getCondition(), dependencies);
        Dependencies bodyDependencies = visit(statement.getStatement());

        bodyDependencies.dependentVariables.removeAll(dependencies.getPotentiallyChangedVariables());
        dependencies.potentiallyChangedVariables.addAll(bodyDependencies.getGuaranteedChangedVariables());
        dependencies.potentiallyChangedVariables.addAll(bodyDependencies.getPotentiallyChangedVariables());
        dependencies.dependentVariables.addAll(bodyDependencies.getDependentVariables());

        Dependencies iterationDependencies = nullableExpression(statement.getIteration(), new Dependencies());
        dependencies.dependentVariables.addAll(iterationDependencies.getDependentVariables());
        dependencies.potentiallyChangedVariables.addAll(iterationDependencies.getGuaranteedChangedVariables());
        dependencies.potentiallyChangedVariables.addAll(iterationDependencies.getPotentiallyChangedVariables());

        dependencies.dependentVariables.removeAll(declaredVariables);
        dependencies.potentiallyChangedVariables.removeAll(declaredVariables);
        dependencies.guaranteedChangedVariables.removeAll(declaredVariables);
        return dependencies;
    }

    private Dependencies visit(IterationStatementWhile statement) {
        Dependencies dependencies = new Dependencies();
        nullableExpression(statement.getCondition(), dependencies);
        Dependencies bodyDependencies = visit(statement.getStatement());
        bodyDependencies.dependentVariables.removeAll(dependencies.getGuaranteedChangedVariables());
        dependencies.potentiallyChangedVariables.addAll(bodyDependencies.getGuaranteedChangedVariables());
        dependencies.potentiallyChangedVariables.addAll(bodyDependencies.getPotentiallyChangedVariables());
        dependencies.dependentVariables.addAll(bodyDependencies.getDependentVariables());
        return dependencies;
    }

    private Dependencies visit(IterationStatementDoWhile statement) {
        Dependencies dependencies = new Dependencies();
        nullableExpression(statement.getCondition(), dependencies);
        Dependencies bodyDependencies = visit(statement.getStatement());
        dependencies.add(bodyDependencies);
        return dependencies;
    }

    private Dependencies visit(ExpressionStatement statement) {
        return visit(statement.getExpression());
    }

    private Dependencies visit(Expression expression) {
        Set<String> dependentVariables = expression.getDependentVariables();
        Set<String> changedVariables = expression.getGuaranteedChangedVariables();
        Set<String> potentialVariables = expression.getPotentiallyChangedVariables();
        return new Dependencies(dependentVariables, changedVariables, potentialVariables);
    }

    private Dependencies visit(CompoundStatement statement) {
        Map<String, Collection<BlockItem>> lastAssigned = new HashMap<>();
        Set<String> incomingValues = new HashSet<>();
        for (BlockItem blockItem : statement.getBlockItems()) {
            if (blockItem instanceof Statement) {
                System.out.println(blockItem.toCode());
                Dependencies dependencies;
                if (blockItem instanceof JumpStatementStrict) {
                    dependencies = new Dependencies(lastAssigned.keySet(), Collections.emptySet(), Collections.emptySet());
                } else {
                    dependencies = visit(((Statement) blockItem));
                }
                System.out.println(dependencies);
                for (String usedVariable : dependencies.getDependentVariables()) {
                    if (!lastAssigned.containsKey(usedVariable)) {
                        incomingValues.add(usedVariable);
                    } else {
                        Collection<BlockItem> froms = lastAssigned.get(usedVariable);
                        BlockItem to = blockItem;
                        for (BlockItem from : froms) {
                            link(from, to);
                        }
                    }
                }
                for (String changedVariables : dependencies.getGuaranteedChangedVariables()) {
                    lastAssigned.put(changedVariables, Sets.newHashSet(blockItem));
                }
                for (String potentiallyChangedVariable : dependencies.getPotentiallyChangedVariables()) {
                    if (!lastAssigned.containsKey(potentiallyChangedVariable)) {
                        lastAssigned.put(potentiallyChangedVariable, new ArrayList<>());
                    }
                    lastAssigned.get(potentiallyChangedVariable).add(blockItem);
                }
                if (blockItem instanceof JumpStatementStrict) {
                    return new Dependencies(incomingValues, Collections.emptySet(), Collections.emptySet());
                }
            } else if (blockItem instanceof Declaration) {
                Declaration declaration = (Declaration) blockItem;
                for (Declaration.DeclaredVariable declaredVariable : declaration.getDeclaredVariables()) {
                    lastAssigned.put(declaredVariable.getIdentifier(), Sets.newHashSet(declaration));
                    if (declaredVariable.getInitializer() != null) {
                        Expression initializer = declaredVariable.getInitializer();
                        Set<String> rightVariables = initializer.getDependentVariables();
                        rightVariables.removeAll(lastAssigned.keySet());
                        incomingValues.addAll(rightVariables);
                        for (String lvalue : initializer.getGuaranteedChangedVariables()) {
                            lastAssigned.put(lvalue, Sets.newHashSet(declaration));
                        }
                    }
                }
            }
        }
        return new Dependencies(incomingValues, lastAssigned.keySet(), Collections.emptySet());
    }

    private Dependencies visit(SelectionStatementIf statement) {
        Dependencies conditionDependencies = visit(statement.getCondition());
        Set<String> dependentVariables = new HashSet<>(conditionDependencies.getDependentVariables());
        Set<String> guaranteedChangedVariables = new HashSet<>(conditionDependencies.getGuaranteedChangedVariables());
        Set<String> potentiallyChangedVariables = new HashSet<>(conditionDependencies.getPotentiallyChangedVariables());

        Statement thenStatement = statement.getThenStatement();
        Dependencies thenDependencies = visit(thenStatement);
        thenDependencies.dependentVariables.removeAll(guaranteedChangedVariables);
        dependentVariables.addAll(thenDependencies.dependentVariables);
        potentiallyChangedVariables.addAll(thenDependencies.getPotentiallyChangedVariables());

        if (statement.getElseStatement() != null) {
            Set<String> branchesChangedVariables = new HashSet<>();
            Dependencies elseDependencies = visit(statement.getElseStatement());
            potentiallyChangedVariables.addAll(elseDependencies.getPotentiallyChangedVariables());
            elseDependencies.dependentVariables.removeAll(guaranteedChangedVariables);
            dependentVariables.addAll(elseDependencies.dependentVariables);
            branchesChangedVariables.addAll(elseDependencies.getGuaranteedChangedVariables());
            branchesChangedVariables.retainAll(thenDependencies.getGuaranteedChangedVariables());
            guaranteedChangedVariables.addAll(branchesChangedVariables);
        } else {
            potentiallyChangedVariables.addAll(thenDependencies.getGuaranteedChangedVariables());
        }
        return new Dependencies(dependentVariables, guaranteedChangedVariables, potentiallyChangedVariables);

    }

    private void printLastAssigned(Map<String, BlockItem> map) {
        for (Map.Entry<String, BlockItem> stringBlockItemEntry : map.entrySet()) {
            System.out.println(stringBlockItemEntry.getKey() + ": " + stringBlockItemEntry.getValue().toCode());
        }
    }
}
