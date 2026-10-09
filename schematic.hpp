// Copyright (c) October 2026 Félix-Olivier Dumas. All rights reserved.
// Licensed under the terms described in the LICENSE file

#include <iostream>
#include <type_traits>
#include <concepts>
#include <vector>




namespace exotic::internal::nttp_identity {

    namespace detail {

        template<auto...>
        struct impl;

        template<auto V>
        struct impl<V> {
            static constexpr decltype(V) value = V;
        };

        template<auto... Vs>
        static constexpr impl<Vs...> impl_v = impl<Vs...>::value;

    } // namespace detail

    namespace api {

        template<auto... Vs>
        struct custom : public detail::impl<Vs...> {};

    } // namespace api

} // namespace exotic::internal::nttp_identity




namespace exotic::internal::configuration_blueprint {

    namespace detail {

        template<template<typename...> typename...>
        struct impl;

        template<template<typename> typename Schema, template<typename> typename Model>
        struct impl<Schema, Model> {
            template<typename T>
            using schema = Schema<T>;

            template<typename T>
            using model = Model<T>;
        };

    } // namespace detail

    namespace api {

        template<template<typename...> typename... TTs>
        struct custom : public detail::impl<TTs...> {};

    } // namespace api

} // namespace exotic::internal::configuration_blueprint




namespace exotic::internal::configuration_payload {

    namespace detail {

        template<typename...>
        struct impl;

        template<typename... Ts>
        struct ConfigurationPayload {};

    } // namespace detail

    namespace api {

        template<typename... Ts>
        struct custom : public detail::impl<Ts...> {};

    } // namespace api

} // namespace exotic::internal::configuration_payload




namespace exotic::internal::guarded_configuration_payload {

    namespace detail {

        template<bool, typename...>
        struct impl;

        template<bool Predicate, typename Payload>
        struct impl<Predicate, Payload> {
            static constexpr bool enable = Predicate;
            using payload_type = Payload;
        };

        template<bool Predicate, typename Payload>
        using impl_t = typename impl<Predicate, Payload>::payload_type;

    } // namespace detail

    namespace api {

        template<bool Predicate, typename... Payloads>
        struct custom : public detail::impl<Predicate, Payloads...> {};

        template<typename... Payloads>
        struct pre_enabled : public detail::impl<true, Payloads...> {};

        template<typename... Payloads>
        struct pre_disabled : public detail::impl<false, Payloads...> {};

    } // namespace api

} // namespace exotic::internal::guarded_configuration_payload




namespace exotic::internal::transform_tuple_using {

    namespace detail {

        template<template<typename...> typename...>
        struct impl;

        template<template<typename> typename Metafunction>
        struct impl<Metafunction> {

            template<typename...>
            struct on;

            template<typename... Ts>
            struct on<std::tuple<Ts...>> {
                using type = std::tuple<typename Metafunction<Ts>::type...>;
            };

            template<typename... Ts>
            using on_t = typename on<Ts...>::type;

        };

        template<template<typename> typename Metafunction, typename Tuple>
        using impl_t = typename impl<Metafunction>::template on<Tuple>::type;

    } // namespace detail

    namespace api {

        template<template<typename...> typename... TTs>
        struct custom : public detail::impl<TTs...> {};

        template<template<typename> typename TT, typename... Ts>
        struct simplified : public detail::impl<TT>::template on<Ts...> {};

    } // namespace api

} // namespace exotic::internal::transform_tuple_using




namespace exotic::internal::rebind_to_tuple {

    namespace detail {

        template<typename...>
        struct impl;

        template<template<typename...> typename AnyContainer, typename... Ts>
        struct impl<AnyContainer<Ts...>> {
            using type = std::tuple<Ts...>;
        };

        template<typename... Ts>
        using impl_t = typename impl<Ts...>::type;

    } // namespace detail

    namespace api {

        template<typename... Ts>
        struct custom : public detail::impl<Ts...> {};

    } // namespace api

} // namespace exotic::internal::rebind_to_tuple




namespace exotic::internal::model_binder {

    namespace detail {

        template<template<typename...> typename...>
        struct impl;

        template<template<typename> typename TargetModel>
        struct impl<TargetModel> {
            template<typename...>
            struct as_metafunction;

            template<typename... Args>
            struct as_metafunction<std::tuple<Args...>> {
                using type = TargetModel<std::tuple<Args...>>;
            };

            template<typename... Ts>
            using as_metafunction_t = typename as_metafunction<Ts...>::type;
        };

    } // namespace detail

    namespace api {

        template<template<typename...> typename... TTs>
        struct custom : public detail::impl<TTs...> {};

        template<template<typename> typename Schema, typename... Arguments>
        struct simplified : public detail::impl<Schema>::template as_metafunction<Arguments...> {};

    } // namespace api

} // namespace exotic::internal::model_binder




namespace exotic::internal::resolve_transform_validation {

    namespace detail {

        template<template<typename...> typename...>
        struct impl;

        template<template<typename> typename TargetSchema>
        struct impl<TargetSchema> {
            template<typename...>
            struct as_metafunction;

            template<typename... Args>
            struct as_metafunction<std::tuple<Args...>> {
                using type = exotic::nttp_identity<
                    TargetSchema<std::tuple<Args...>>::valid
                >;
            };

            template<typename... Ts>
            using as_metafunction_t = typename as_metafunction<Ts...>::type;
        };

    } // namespace detail

    namespace api {

        template<template<typename...> typename... TTs>
        struct custom : public detail::impl<TTs...> {};

        template<template<typename> typename Schema, typename... Arguments>
        struct simplified : public detail::impl<Schema>::template as_metafunction<Arguments...> {};

    } // namespace api

} // namespace exotic::internal::resolve_transform_validation




namespace exotic::internal::configuration_validator {

    namespace detail {

        template<template<typename> typename Schema>
        struct impl {
            template<typename... Arguments>
            struct validate_for {
                using type = typename decltype(
                    []<typename... Args>(std::type_identity<Args>...) {

                        using args_as_tuple_t = std::tuple<Args...>;

                        using unzipped_tuple_t =
                            typename exotic::transform_tuple_using<rebind_to_tuple>
                                ::template on_t<args_as_tuple_t>;

                        using validation_results =
                            typename exotic::transform_tuple_using<
                                resolve_transform_validation<Schema>::template as_metafunction
                            >::template on_t<unzipped_tuple_t>;

                        return std::type_identity<validation_results>{};

                    }(std::type_identity<Arguments>{}...)
                )::type;
            };

            template<typename... Arguments>
            using validate_for_t = typename validate_for<Arguments...>::type;
        };

    } // namespace detail

    namespace api {

        template<template<typename...> typename... Schemas>
        struct custom: public detail::impl<Schemas...> {};

        template<template<typename> typename Schema, typename... Arguments>
        struct simplified : public detail::impl<Schema>::template validate_for<Arguments...> {};

    } // namespace api

} // namespace exotic::internal::configuration_validator




namespace exotic::internal::configuration_enforcer {

    namespace detail {

        template<template<auto...> typename...>
        struct impl;

        template<template<auto...> typename Policy>
        struct impl<Policy> {
            template<typename...>
            struct enforce_for; // trouver nouveau nom

            template<bool... Validations> // on passe le tuple complet
            struct enforce_for<std::tuple<exotic::nttp_identity<Validations>...>> {

                static constexpr bool value =
                    []<std::size_t... Is>(std::index_sequence<Is...>) {
                        return (Policy<Is, Validations>::value && ...);
                    }(std::make_index_sequence<sizeof...(Validations)>{});

                static_assert(value, "invalid configuration i guess...");

            };

            template<bool... Validations>
            static constexpr bool enforce_for_t = enforce_for<Validations...>::value;
        };

    } // namespace detail

    namespace api {

        template<template<auto...> typename... Policies>
        struct custom : public detail::impl<Policies...> {};

        template<template<auto...> typename Policy, typename... Arguments>
        struct simplified : public detail::impl<Policy>::template enforce_for<Arguments...> {};

    } // namespace api

} // namespace exotic::internal::configuration_enforcer




namespace exotic::internal::configuration_builder {

    namespace detail {

        template<template<typename...> typename...>
        struct impl;

        template<template<typename> typename Model>
        struct impl<Model> {
            template<typename... Arguments>
            struct build_for {
                using type = typename decltype( // IIFE
                    []<typename... Args>(std::type_identity<Args>...) {

                        using args_as_tuple_t = std::tuple<Args...>;

                        using unzipped_tuple_t =
                            typename exotic::transform_tuple_using<rebind_to_tuple>
                                ::on_t<args_as_tuple_t>;

                        using tuple_of_models =
                            exotic::transform_tuple_using<
                                model_binder<Model>::template as_metafunction
                            >::on_t<unzipped_tuple_t>;

                        return std::type_identity<tuple_of_models>{};

                    }(std::type_identity<Arguments>{}...)
                )::type;
            };

            template<typename... Arguments>
            using build_for_t = typename build_for<Arguments...>::type;
        };

    } // namespace detail

    namespace api {

        template<template<typename...> typename... Models>
        struct custom : public detail::impl<Models...> {};

        template<template<typename...> typename Model, typename... Arguments>
        struct simplified : public detail::impl<Model>::template build_for<Arguments...> {};

    } // namespace api

} // namespace exotic::internal::configuration_builder




namespace exotic::internal::configuration_generator {

    namespace detail {

        template<
            template <template <typename...> typename> typename Validator,
            template <template <auto...>     typename> typename Evaluator,
            template <template <typename...> typename> typename Builder
        >
        struct impl {
        protected:
            template<typename...>
            struct generation_pipeline;

            template<
                template<typename> typename Schema, // concept maybe
                template<typename> typename Model
            > 
            struct generation_pipeline<
                internal::configuration_blueprint::api::custom<Schema, Model>
            > {

                template<typename...>
                struct execute_for;

                template<typename... Payloads>
                struct execute_for {
                    using type = typename decltype(
                        []<typename... Args>(std::type_identity<Args>...) {

                            using validation_results =
                                typename Validator<Schema>::template validate_for<Payloads...>::type;

                            Evaluator<must_be_true_validation_policy>
                                ::template enforce_for<validation_results>::value;

                            using resolved_config =
                                typename Builder<Model>::template build_for<Payloads...>::type;

                            return std::type_identity_t<resolved_config>{};

                        }(std::type_identity<Payloads>{}...)
                    )::type;
                };

                template<typename... Ts>
                using execute_for_t = typename execute_for<Ts...>::type;

            };

        public:
            template<typename...>
            struct generate_using;

            template<typename Blueprint>
            struct generate_using<Blueprint> {

                template<typename...>
                struct over;

                template<typename... Payloads>
                struct over {
                    using type = typename generation_pipeline<Blueprint>
                                    ::template execute_for<Payloads...>::type;
                };
            };
        };

    } // namespace detail

    namespace api {

        template <
            template <template <typename...> typename> typename Validator,
            template <template <auto...>     typename> typename Evaluator,
            template <template <typename...> typename> typename Builder
        >
        struct custom : detail::impl<Validator, Evaluator, Builder> {};


        // peut etre faire genre default pour les trois modules injectés


        template <
            template <template <typename...> typename> typename Validator = internal::configuration_validator::api::custom,
            template <template <auto...>     typename> typename Evaluator = internal::configuration_enforcer::api::custom,
            template <template <typename...> typename> typename Builder   = internal::configuration_builder::api::custom
        >
        struct with : detail::impl<Validator, Evaluator, Builder> {};


        struct standard : detail::impl<
            internal::configuration_validator::api::custom,
            internal::configuration_enforcer::api::custom,
            internal::configuration_builder::api::custom
        > {};

        // genre faire une version avec l'api Guarded (ou faire autre type idk)

    } // namespace api

} // namespace exotic::internal::configuration_generator




namespace exotic {

    template<auto... Vs>
    using nttp_identity = internal::nttp_identity::api::custom<Vs...>;



    template<template<typename...> typename... TTs>
    using configuration_blueprint = internal::configuration_blueprint::api::custom<TTs...>;



    template<typename... Ts>
    using configuration_payload = internal::configuration_payload::api::custom<Ts...>;

    template<auto V, typename... Ts>
    using guarded_configuration_payload = internal::guarded_configuration_payload::api::custom<V, Ts...>;



    template<template<typename...> typename... TTs>
    using transform_tuple_using = internal::transform_tuple_using::api::custom<TTs...>;

    template<typename... Ts>
    using rebind_to_tuple = internal::rebind_to_tuple::api::custom<Ts...>;

    template<template<typename...> typename... TTs>
    using model_binder = internal::model_binder::api::custom<TTs...>;



    template<template<typename...> typename... TTs>
    using resolve_transform_validation = internal::resolve_transform_validation::api::custom<TTs...>;



    template<template<typename...> typename... TTs>
    using configuration_validator = internal::configuration_validator::api::custom<TTs...>;

    template<template<auto...> typename... TTs>
    using configuration_enforcer = internal::configuration_enforcer::api::custom<TTs...>;

    template<template<typename...> typename... TTs>
    using configuration_builder = internal::configuration_builder::api::custom<TTs...>;



    using configuration_generator = internal::configuration_generator::api::standard;

} // namespace exotic
